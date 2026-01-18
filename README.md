# Smart Ventilation System

## Overview

The **Smart Ventilation System** is an IoT-based solution aimed at improving indoor air quality and enhancing energy efficiency. The system leverages real-time data and predicted particulate matter (PM2.5) levels to send ventilation control commands. It is designed to make decisions based on a combination of a sensor, API, and data analysis. Due to hardware limitations, it currently uses a buzzer to emit different signals instead of more complex actuators.


# Project Structure

## Folders

### `data_collection_phase/`
This folder contains the files related to the initial data collection phase of the project. It contains the python and C++ code used to log API data and sensor data over 8 days, as well as the python code that was used to analyse the data and develop the PM2.5 prediction model.

### `templates/`
Contains the index.html template file used by the Flask web application. This template is rendered by the web app to display dynamic content to the user.

### `Connect_webapp_and_ESP32/`
This folder contains the C++ code that facilitates communication between the ESP32 microcontroller and the web application via MQTT. The code allows the microcontroller to send sensor data to the web app, while also receiving commands from the web app to control the buzzer based on air quality decisions.  

## Files

### `app.py`
The main Flask application that controls the web server. It handles requests, renders templates, communicates with the ESP32 via MQTT, and displays real-time data and analytics on the web interface.

### `random_forest_model.joblib`
A trained random forest machine learning model saved using the `joblib` library. This model is used to predict air quality and help in decision-making regarding ventilation control.

### `requirements.txt`
This file lists the Python libraries required to run the project. It allows users to easily install the necessary dependencies using `pip`.

### `utils.py`
A Python utility script that contains helper functions used by the `app.py` script.  



## Features
- **Air Quality Monitoring**: The system collects data from both indoor sensor and API to monitor air quality in real-time. This includes indoor and outdoor temperature and humidity, outdoor particulate matter (PM2.5, PM10), CO2, NO2 and O3 levels, and wind speed.
  
- **Automated Control**: Based on the collected data and the predicted PM2.5 concentration, the system makes real-time decisions on whether to open or close windows to optimize air quality and temperature.  

- **Healthier environment**: The system aims to reduce pollutants that affect indoor air quality.


## How It Works

<p align="center">
  <img alt="setup diagram" src="setup.png" width="700" height="600" />
</p>

- The first ESP32 reads data from the sensor and sends it to the second ESP32 via LoRa.
- The second ESP32 sends the sensor data to the web app via MQTT.
- The web app processes the sensor data, fetches additional data from the weather API, predicts future PM2.5 levels, and makes decisions on how to control the ventilation.
- The decisions are sent back to the second ESP32, which uses a buzzer to signal appropriate actions based on the web app's feedback.


<p align="center">
  <img alt="Actual setup images" src="actual_setup_images.png" width="700" height="600" />
</p>

## System Components

1. **DHT11 Sensor**  
  
2. **Two ESP32 Microcontrollers**  

3. **Flask Web App**  

5. **Buzzer**  

 
## Use Cases
- **Smart Homes**: homeowners who want to optimize indoor air quality and energy efficiency while reducing reliance on HVAC systems.
- **Nursing Homes**: nursing homes and elderly care facilities, where maintaining a clean and healthy indoor environment is crucial for residents' health and comfort.
- **Offices**: Businesses that want to enhance employee health and comfort by improving air quality and energy use efficiency.
- **Health-Related Applications**: Particularly useful for individuals with respiratory issues or allergies, as it could be used to automatically ensure a healthier indoor environment by controlling ventilation based on real-time air quality data.


## Conclusion
The Smart Ventilation System optimizes indoor air quality by leveraging real-time data analysis and automation. Through this IoT setup, ventilation can be controlled efficiently, ensuring a healthier indoor environment and minimal energy usage.




 

