



import pandas as pd
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.by import By
from selenium import webdriver
from selenium.webdriver.edge.service import Service as EdgeService
import time
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC


# To check what day the Requested Day is
def check_day(row, row_day1, row_day2, row_day3):
    if row_day1 == 'Saturday' or row_day1 == 'Sunday':
        print("W E E K E N D")
        print(f"{row['Requested Tour Date- Option 1 ']}   {row['Requested Tour Time- Option 1']}    {row_day1}\n")
        print(f"{row['Requested Tour Date- Option 2']}   {row['Requested Tour Time- Option 2']}     {row_day2}\n"
              f"{row['Requested Tour Date- Option 3']}   {row['Requested Tour Time- Option 3']}     {row_day3}\n\n")
    elif row_day2 == 'Saturday' or row_day2 == 'Sunday':
        print(f"{row['Requested Tour Date- Option 1 ']}   {row['Requested Tour Time- Option 1']}     {row_day1}\n")
        print("W E E K E N D")
        print(f"{row['Requested Tour Date- Option 2']}   {row['Requested Tour Time- Option 2']}     {row_day2}\n")
        print(f"{row['Requested Tour Date- Option 3']}   {row['Requested Tour Time- Option 3']}     {row_day3}\n\n")
    elif row_day3 == 'Saturday' or row_day3 == 'Sunday':
        print(f"{row['Requested Tour Date- Option 1 ']}   {row['Requested Tour Time- Option 1']}     {row_day1}\n"
              f"{row['Requested Tour Date- Option 2']}   {row['Requested Tour Time- Option 2']}     {row_day2}\n")
        print("W E E K E N D")
        print(f"{row['Requested Tour Date- Option 3']}   {row['Requested Tour Time- Option 3']}     {row_day3}\n\n")
    else:
        print(f"{row['Requested Tour Date- Option 1 ']}   {row['Requested Tour Time- Option 1']}     {row_day1}\n"
              f"{row['Requested Tour Date- Option 2']}   {row['Requested Tour Time- Option 2']}     {row_day2}\n"
              f"{row['Requested Tour Date- Option 3']}   {row['Requested Tour Time- Option 3']}     {row_day3}\n\n")


# Finds the desired element using its xpath on the webpage & a special case for Forms
def find_element(driver, xpath, mode):
    if mode:
        i = 0
        while i < 5:
            try:
                # Wait for the element to be present
                element = WebDriverWait(driver, 5).until(
                    EC.presence_of_element_located((By.XPATH, xpath)))
                if element:
                    element.click()
                    print("Login successful!")
                    time.sleep(5)
                    return
            except:
                print("Retrying Login ..")
                driver.refresh()
                i += 1
                time.sleep(2)
        print("The element cannot be located\n")
    else:
        elements = driver.find_elements(By.XPATH, '//*')
        keyword = 'forms'
        for element in elements:
            if keyword in element.text.lower():
                print("Forms found")
                element.click()
                time.sleep(5)
                return
        print("Forms cannot be located")

# Checks the Date and returns the Week Day Name
def day_name(row, T):
    row_date = pd.to_datetime(row[T])
    row_day = row_date.day_name()
    return row_day


# Initialize the driver
driver_path = "C:/Data/Sujal Documents/Sujal Documents Final/BOT/Tools/msedgedriver.exe"  # "C:/Program Files/Google/Chrome/Application/chrome.exe"
service = EdgeService(driver_path)
driver = webdriver.Edge(service=service)

# Open up MyApps and Navigate to Microsoft Forms
try:
    driver.get('https://www.google.com')

    time.sleep(5)

    search_box = driver.find_element(By.NAME, 'q')
    my_apps = 'https://myapplications.microsoft.com/'
    search_box.send_keys(my_apps)
    search_box.send_keys(Keys.RETURN)

    first_result = driver.find_element(By.XPATH, '(//h3)[1]')
    first_result.click()

    # Locate and click on the Email Profile for login
    find_element(driver, '//*[@id="tilesHolder"]/div[1]/div/div', 1)
    # Locate Forms
    find_element(driver, 'N/A', 0)

    driver.get(
        'https://forms.office.com/Pages/DesignPageV2.aspx?auth_pvr=OrgId&auth_upn=sujal.more@uta.edu&origin=shell')
    time.sleep(5)

    driver.execute_script("window.open('https://outlook.office.com/mail/be.an.engineer@uta.edu/');")
    time.sleep(5)

    # Asks User to input File Path; Raw input is acceptable
    file_path = input("Enter the file path: ").strip().strip('"')  # 'C:/Users/sujal/Downloads/Test.xlsx'
    file_path = file_path.replace("\\", "/")
    df = pd.read_excel(file_path)

    # Excel file processing
    total_rows = len(df)
    latest_row = df.iloc[total_rows - 1]
    print(f"Total Rows: {total_rows}")
    print(
        f"Most recent Entry: \nStart Time: {latest_row['Start time']}    Completion Time: {latest_row['Completion time']}\n")

    if total_rows > 2:
        target_index = total_rows - 3
        row1 = df.iloc[target_index]

        print("-" * 100)
        print(f"ID: {row1['ID']} \n"
              f"Name First Last 1: {row1['First Name']} {row1['Last Name']}\n"
              f"Email: {row1['Email2']}\n"
              f"DOB: {row1['Date of Birth']} \n"
              f"Application Type & Majors: {row1['Application type']}    {row1['Major(s) of Interest- Select no more than 3 engineering majors of interest.']}\n"
              f"Format of Tour: {row1['Format for Tour']}\n"
              f"Requested Dates & Times: \n")

        row1_day1 = day_name(row1, 'Requested Tour Date- Option 1 ')
        row1_day2 = day_name(row1, 'Requested Tour Date- Option 2')
        row1_day3 = day_name(row1, 'Requested Tour Date- Option 3')
        check_day(row1, row1_day1, row1_day2, row1_day3)

        target_index2 = total_rows - 2
        row2 = df.iloc[target_index2]
        print("-" * 100)
        print(f"ID: {row2['ID']} "
              f"Name First Last 2: {row2['First Name']} {row2['Last Name']} "
              f"Email: {row2['Email2']}\n"
              f"DOB: {row2['Date of Birth']} \n"
              f"Application Type & Majors: {row2['Application type']}    {row2['Major(s) of Interest- Select no more than 3 engineering majors of interest.']}\n"
              f"Format of Tour: {row2['Format for Tour']}\n"
              f"Requested Dates & Times: \n")

        row2_day1 = day_name(row2, 'Requested Tour Date- Option 1 ')
        row2_day2 = day_name(row2, 'Requested Tour Date- Option 2')
        row2_day3 = day_name(row2, 'Requested Tour Date- Option 3')
        check_day(row2, row2_day1, row2_day2, row2_day3)

        target_index3 = total_rows - 1
        print("-" * 100)
        row3 = df.iloc[target_index3]
        print(f"ID: {row3['ID']} "
              f"Name First Last 3: {row3['First Name']} {row3['Last Name']}\n"
              f"Email: {row3['Email2']}\n"
              f"DOB: {row3['Date of Birth']} \n"
              f"Application Type & Majors: {row3['Application type']}    {row3['Major(s) of Interest- Select no more than 3 engineering majors of interest.']}\n"
              f"Format of Tour: {row3['Format for Tour']}\n"
              f"Requested Dates & Times: \n")

        row3_day1 = day_name(row3, 'Requested Tour Date- Option 1 ')
        row3_day2 = day_name(row3, 'Requested Tour Date- Option 2')
        row3_day3 = day_name(row3, 'Requested Tour Date- Option 3')
        check_day(row3, row3_day1, row3_day2, row3_day3)

    # Makes sure the webpage stays open
    while (True):
        decision = input("Do you want to Exit? Y/N ")
        if decision == 'Y' or 'y':
            break
        else:
            time.sleep(2)

except Exception as e:
    print(f"An error occurred: {e}")
finally:
    driver.quit()
