#include "service_manager.h"
#include "core/log.h"

static const char *TAG = "ServiceManager";

ServiceManager& ServiceManager::instance() {
    static ServiceManager instance;
    return instance;
}

ServiceManager::ServiceManager() 
    : coreServices_(CoreServices::instance()),
      connectivityServices_(ConnectivityServices::instance()) {
}

bool ServiceManager::initializeCore() {
    if (coreInitialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing core services...");
    
    if (!coreServices_.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize core services");
        return false;
    }
    
    coreInitialized_ = true;
    ESP_LOGI(TAG, "Core services initialized successfully");
    return true;
}

void ServiceManager::startConnectivityInitialization() {
    if (connectivityStarted_) {
        return;
    }
    
    ESP_LOGI(TAG, "Starting connectivity services initialization...");
    connectivityServices_.startAsyncInitialization();
    connectivityStarted_ = true;
}

void ServiceManager::shutdown() {
    ESP_LOGI(TAG, "Shutting down service manager...");
    
    connectivityServices_.shutdown();
    coreServices_.shutdown();
    
    coreInitialized_ = false;
    connectivityStarted_ = false;
}

bool ServiceManager::isCoreReady() const {
    return coreInitialized_ && coreServices_.isReady();
}

bool ServiceManager::isConnectivityReady() const {
    return connectivityStarted_ && connectivityServices_.isReady();
}

bool ServiceManager::isSystemReady() const {
    return isCoreReady() && isConnectivityReady();
}

// Legacy compatibility method
bool ServiceManager::initialize() {
    if (!initializeCore()) {
        return false;
    }
    
    startConnectivityInitialization();
    return true;
}

// Core services access
AudioManager& ServiceManager::audio() {
    return coreServices_.audio();
}

DisplayManager& ServiceManager::display() {
    return coreServices_.display();
}

KeyProcessor& ServiceManager::keyProcessor() {
    return coreServices_.keyProcessor();
}

// Connectivity services access
BLEKeyboard& ServiceManager::bleKeyboard() {
    return connectivityServices_.bleKeyboard();
}

WiFiControl& ServiceManager::wifi() {
    return connectivityServices_.wifi();
}
