"""
 * Personal License
 *
 * Author: Sujal More
 * Project: Cloud Inventory Management App
 *
 * This code is provided solely for educational and
 * personal use. Unauthorized copying, distribution,
 * or commercial use of this code, in whole or in
 * part, without the explicit permission of the author
 * is strictly prohibited.
 *
 * For permissions or inquiries, please contact:
 * sujalm7200@gmail.com
 *
 * © 2025 Sujal More. All rights reserved.
"""


import streamlit as st
from pymongo import MongoClient
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa, padding, dh
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.backends import default_backend
import secrets, string, os

# MongoDB Setup
MONGO_URI = st.secrets["MONGO_URI"]
client = MongoClient(MONGO_URI)
db = client["STEG"]
users_col = db["users"]

# Session State
if "user" not in st.session_state:
    st.session_state.user = None


# Auth Functions
def login(username, password):
    user = users_col.find_one({"username": username})
    if user and user["password"] == password:
        st.session_state.user = username
        return True
    return False


def register(username, password):
    if users_col.find_one({"username": username}):
        return False
    users_col.insert_one({"username": username, "password": password})
    return True


def logout():
    st.session_state.user = None


# === Layout ===
st.title("🔐 Cryptographic Utility Platform")

if not st.session_state.user:
    st.subheader("Login")
    user = st.text_input("Username")
    pw = st.text_input("Password", type="password")
    if st.button("Login"):
        if login(user, pw):
            st.success("Logged in!")
            st.rerun()
        else:
            st.error("Invalid credentials")

    st.subheader("Register")
    new_user = st.text_input("New Username", key="new_user")
    col1, col2 = st.columns(2)
    with col1:
        new_pw = st.text_input("New Password", type="password", key="new_pw")
    with col2:
        if st.button("Generate Secure Password"):
            import string

            charset = string.ascii_letters + string.digits + string.punctuation
            secure_pw = ''.join(secrets.choice(charset) for _ in range(16))
            st.session_state.generated_pw = secure_pw

    if "generated_pw" in st.session_state:
        st.code(st.session_state.generated_pw, language="text")
        st.download_button("Download Password", st.session_state.generated_pw.encode(), file_name="secure_password.txt")
        new_pw = st.session_state.generated_pw

    if st.button("Register"):
        if register(new_user, new_pw):
            st.success("User registered!")
            st.session_state.pop("generated_pw", None)
            st.session_state.pop("new_user", None)
            st.session_state.pop("new_pw", None)
            st.rerun()
        else:
            st.error("Username already exists.")
    st.stop()

if st.button("Logout"):
    logout()
    st.rerun()

st.markdown(f"**Logged in as:** `{st.session_state.user}`")

# === Tabs ===
tabs = st.tabs([
    "🔐 Encryption",
    "🔓 Decryption",
    "🔍 Secure Hash",
    "🔑 Key Generation & Sharing",
    "🔁 Compare Hashes"
])

# --- Encryption ---
with tabs[0]:
    st.header("🔐 Encrypt Files (3DES / AES / RSA)")
    method = st.selectbox("Encryption Method", ["3DES", "AES", "RSA"])
    uploaded_file = st.file_uploader("Upload File to Encrypt", key="enc")
    if uploaded_file:
        file_data = uploaded_file.read()

        if method in ["3DES", "AES"]:
            # === AES Key Size Selector ===
            if method == "AES":
                if "aes_key_size" not in st.session_state:
                    st.session_state.aes_key_size = 16

                st.session_state.aes_key_size = st.selectbox(
                    "AES Key Size (bytes)", [16, 24, 32],
                    index=[16, 24, 32].index(st.session_state.aes_key_size),
                    key="aes_key_size_select"
                )

            # === Generate Key ===
            if st.button("Generate Random Key", key="gen_key_btn"):
                if method == "3DES":
                    key_parts = [secrets.token_bytes(8) for _ in range(3)]  # K1, K2, K3
                    key = b"".join(key_parts)
                    st.markdown("Using 3-key 3DES: 3 independent 8-byte keys (K1, K2, K3)")
                else:
                    size = st.session_state.aes_key_size
                    key = secrets.token_bytes(size)

                st.session_state.generated_key = key.hex()

            # === Show Generated Key ===
            if "generated_key" in st.session_state:
                st.code(st.session_state.generated_key)
                st.download_button("Download Key", st.session_state.generated_key.encode(),
                                   file_name=f"{method.lower()}_key.bin")

            # === IV Section (AES Only) ===
            if method == "AES":
                if st.button("Generate IV (16 bytes)", key="gen_iv_btn"):
                    iv = secrets.token_bytes(16)
                    st.session_state.generated_iv = iv.hex()

                iv_hex = st.text_input(
                    "IV (hex, 16 bytes)",
                    value=st.session_state.get("generated_iv", "00000000000000000000000000000000"),
                    key="aes_iv_hex"
                )
            else:
                iv_hex = st.text_input("IV (hex, 8 bytes)", value="0000000000000000", key="des_iv_hex")

            # === Manual Key + Block Mode ===
            key_hex = st.text_input("Key (hex)", key="enc_key_hex")
            block_mode = st.selectbox("Mode", ["ECB", "CBC"], key="enc_block_mode")

            if st.button("Encrypt", key="encrypt_btn"):
                try:
                    key = bytes.fromhex(key_hex)
                    iv = bytes.fromhex(iv_hex)
                    algo = algorithms.TripleDES(key) if method == "3DES" else algorithms.AES(key)
                    mode = modes.ECB() if block_mode == "ECB" else modes.CBC(iv)
                    cipher = Cipher(algo, mode, backend=default_backend())
                    pad_len = algo.block_size // 8 - len(file_data) % (algo.block_size // 8)
                    padded = file_data + bytes([pad_len] * pad_len)
                    result = cipher.encryptor().update(padded) + cipher.encryptor().finalize()
                    st.download_button("Download Encrypted File", result, file_name="encrypted.bin")
                except Exception as e:
                    st.error(f"Error: {e}")
        elif method == "RSA":
            st.markdown("### 🔐 RSA Encryption")
            pub_key = st.file_uploader("Upload Public Key (.pem)", key="rsa_enc_encrypt")

            if pub_key:
                if st.button("Encrypt with RSA"):
                    try:
                        public_key = serialization.load_pem_public_key(pub_key.read())
                        result = public_key.encrypt(
                            file_data,
                            padding.OAEP(
                                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                                algorithm=hashes.SHA256(),
                                label=None
                            )
                        )
                        st.download_button("Download Encrypted File", result, file_name="rsa_encrypted.bin")
                    except Exception as e:
                        st.error(f"Error: {e}")
            else:
                st.info("Please upload a public key (.pem) to enable RSA encryption.")

# --- Decryption ---
with tabs[1]:
    st.header("🔓 Decrypt Files (3DES / AES / RSA)")
    method = st.selectbox("Decryption Method", ["3DES", "AES", "RSA"])
    uploaded_file = st.file_uploader("Upload File to Decrypt", key="dec")
    if uploaded_file:
        file_data = uploaded_file.read()

        if method in ["3DES", "AES"]:
            key_hex = st.text_input("Key (hex)")
            iv_hex = st.text_input("IV (hex)", value="0000000000000000")
            block_mode = st.selectbox("Mode", ["ECB", "CBC"])
            if st.button("Decrypt"):
                try:
                    key = bytes.fromhex(key_hex)
                    iv = bytes.fromhex(iv_hex)
                    algo = algorithms.TripleDES(key) if method == "3DES" else algorithms.AES(key)
                    mode = modes.ECB() if block_mode == "ECB" else modes.CBC(iv)
                    cipher = Cipher(algo, mode, backend=default_backend())
                    decrypted = cipher.decryptor().update(file_data) + cipher.decryptor().finalize()
                    pad_len = decrypted[-1]
                    result = decrypted[:-pad_len]
                    st.download_button("Download Decrypted File", result, file_name="decrypted.txt")
                except Exception as e:
                    st.error(f"Error: {e}")

        elif method == "RSA":
            st.markdown("### 🔓 RSA Decryption")
            priv_key = st.file_uploader("Upload Private Key (.pem)", key="rsa_dec_decrypt")
    
            if priv_key:
                if st.button("Decrypt with RSA"):
                    try:
                        private_key = serialization.load_pem_private_key(priv_key.read(), password=None)
                        result = private_key.decrypt(
                            file_data,
                            padding.OAEP(
                                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                                algorithm=hashes.SHA256(),
                                label=None
                            )
                        )
                        st.download_button("Download Decrypted File", result, file_name="rsa_decrypted.txt")
                    except Exception as e:
                        st.error(f"Error: {e}")
            else:
                st.info("Please upload your private key (.pem) to decrypt.")

# --- Secure Hash ---
with tabs[2]:
    st.header("🔍 Secure Hashing (SHA-3)")
    algo = st.selectbox("Hash Algorithm", ["SHA3-256"])
    file = st.file_uploader("Upload File to Hash", key="hash")
    if file and st.button("Generate Hash"):
        file_data = file.read()
        hash_func = getattr(hashes, algo.replace("-", "_"))()
        digest = hashes.Hash(hash_func)
        digest.update(file_data)
        result = digest.finalize().hex()
        st.code(result)

# --- Compare Hashes ---
with tabs[4]:
    st.header("🔁 Compare Hashes")

    mode = st.radio("Comparison Mode", ["File vs File", "File vs Manual Hash"])

    algo = st.selectbox("Hash Algorithm", ["SHA3-256"], key="cmp_algo")

    if mode == "File vs File":
        file1 = st.file_uploader("Upload First File", key="cmp1")
        file2 = st.file_uploader("Upload Second File", key="cmp2")

        if file1 and file2 and st.button("Compare Files"):
            data1 = file1.read()
            data2 = file2.read()
            hash_func = getattr(hashes, algo.replace("-", "_"))()

            h1 = hashes.Hash(hash_func)
            h1.update(data1)
            digest1 = h1.finalize().hex()

            h2 = hashes.Hash(hash_func)
            h2.update(data2)
            digest2 = h2.finalize().hex()

            st.code(f"File 1 Hash: {digest1}")
            st.code(f"File 2 Hash: {digest2}")

            if digest1 == digest2:
                st.success("✅ Hashes match! The files are identical.")
            else:
                st.error("❌ Hashes do NOT match. The files are different.")

    else:  # File vs Manual Hash
        file = st.file_uploader("Upload File to Hash", key="cmp_file")
        expected_hash = st.text_input("Enter Expected Hash (hex)")

        if file and expected_hash and st.button("Compare Hash"):
            data = file.read()
            hash_func = getattr(hashes, algo.replace("-", "_"))()

            h = hashes.Hash(hash_func)
            h.update(data)
            digest = h.finalize().hex()

            st.code(f"Calculated File Hash: {digest}")
            st.code(f"Provided Expected Hash: {expected_hash}")

            if digest.lower() == expected_hash.lower():
                st.success("✅ Hash matches! File integrity verified.")
            else:
                st.error("❌ Hash does NOT match. File may be corrupted or altered.")

# --- Key Generation & Sharing ---
with tabs[3]:
    st.header("🔑 Key Generation and Sharing")
    gen_option = st.radio("Generate", ["RSA-2048", "DH Key Exchange"])

    if gen_option == "RSA-2048":
        if st.button("Generate RSA Keys", key="gen_rsa_btn"):
            private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
            public_key = private_key.public_key()

            st.session_state.generated_rsa_private = private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption()
            )
            st.session_state.generated_rsa_public = public_key.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo
            )

    # Show download buttons if keys exist
    if "generated_rsa_private" in st.session_state and "generated_rsa_public" in st.session_state:
        st.download_button("Download Private Key", st.session_state.generated_rsa_private, file_name="private.pem")
        st.download_button("Download Public Key", st.session_state.generated_rsa_public, file_name="public.pem")

    if gen_option == "DH Key Exchange" and st.button("Generate DH Pair"):
        parameters = dh.generate_parameters(generator=2, key_size=512)
        private_key = parameters.generate_private_key()
        public_key = private_key.public_key()
        priv_bytes = private_key.private_bytes(encoding=serialization.Encoding.PEM,
                                               format=serialization.PrivateFormat.PKCS8,
                                               encryption_algorithm=serialization.NoEncryption())
        pub_bytes = public_key.public_bytes(encoding=serialization.Encoding.PEM,
                                            format=serialization.PublicFormat.SubjectPublicKeyInfo)
        st.download_button("Private Key", priv_bytes, file_name="dh_private.pem")
        st.download_button("Public Key", pub_bytes, file_name="dh_public.pem")

    st.subheader("Compute Shared Key")
    priv_dh = st.file_uploader("Upload Your DH Private Key", key="dh_priv")
    pub_dh = st.file_uploader("Upload Other's DH Public Key", key="dh_pub")
    if priv_dh and pub_dh and st.button("Compute Shared Key"):
        private_key = serialization.load_pem_private_key(priv_dh.read(), password=None)
        peer_public = serialization.load_pem_public_key(pub_dh.read())
        shared = private_key.exchange(peer_public)
        derived = HKDF(algorithm=hashes.SHA256(), length=32, salt=None, info=b'handshake',
                       backend=default_backend()).derive(shared)
        st.code(f"Derived Key: {derived.hex()}")
