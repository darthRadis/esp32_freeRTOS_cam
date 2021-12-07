PROJECT_NAME := esp32-cam-ota
ESP32_IP := 192.168.0.81

include $(IDF_PATH)/make/project.mk

ota: app
	curl ${ESP32_IP}:8032 --data-binary @- < build/${PROJECT_NAME}.bin

