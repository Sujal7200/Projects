# Cloud-Based Inventory Management App

This is a cloud-based inventory management system that allows users to track and manage items across multiple physical locations. The application is built using **Streamlit**, **Python**, and **MongoDB** to provide real-time tracking, a user-friendly interface, and scalable backend performance.

## Features

- **User Authentication:** Secure login system to ensure only authorized users can access the inventory.
- **Inventory Management:** Supports managing items across multiple locations with real-time updates and data synchronization.
- **CRUD Operations:** Create, read, update, and delete inventory items with an intuitive user interface.
- **Custom Item IDs:** Automatically generate unique, human-readable item IDs based on item name and location for easy identification.
- **Responsive UI:** A dynamic UI that auto-fills forms and updates inventory in real-time.

## Technologies Used

- **Python**  
- **Streamlit**  
- **MongoDB**  
- **PyMongo**

## Access the app:
`` esainventory.streamlit.app ``

## Features Walkthrough
1. Login: Users must log in with their credentials to access the app.Authentication is handled via username and password stored in a MongoDB users collection.

2. View Inventory: Displays a list of items across different locations (e.g., XX323, XX324, XX325). Users can view the quantity and name of each item in real-time.

3. Add Item: Allows users to add new items to the inventory with customizable item names, quantities, and locations. Custom item IDs are generated automatically based on the name and location.

4. Update Item: Users can update item details, including name, quantity, and location. If the item ID changes, the system ensures no duplication occurs.

License
This project is licensed under the MIT License - see the Source file for details.
