#include <iostream>
#include "cfg_parser.hpp"

int main() {
    try {
        //----------------------------------------------------
        // Initialize the global configuration with the main config file
        cfgparser::initConfig("example.cfg");
        
        // Get the global config instance
        auto config = cfgparser::getConfig();
        
        //----------------------------------------------------
        // an alternative way
        // using local variable
        // auto config = cfgparser::Config("example.cfg");
        //----------------------------------------------------

        // Access settings from the main (unordered) section
        std::cout << "Setting A: " << config->get("setting_a") << std::endl;
        std::cout << "Setting B: " << config->get("setting_b") << std::endl;
        std::cout << "Setting C: " << config->get("setting_c") << std::endl;
        std::cout << "File Path: " << config->get("file_path") << std::endl;

        // Access settings from the 'DatabaseConnection' section
        std::cout << "\nDatabase Connection Settings:" << std::endl;
        std::cout << "Host: " << config->get("DatabaseConnection", "db_host") << std::endl;
        std::cout << "Port: " << config->get("DatabaseConnection", "db_port") << std::endl;
        std::cout << "User: " << config->get("DatabaseConnection", "db_user") << std::endl;
        std::cout << "Password: " << config->get("DatabaseConnection", "db_password") << std::endl;
        std::cout << "Database Name: " << config->get("DatabaseConnection", "db_name") << std::endl;

        // Access settings from the 'ServiceLimits' section (included from defaults.cfg)
        std::cout << "\nService Limits:" << std::endl;
        std::cout << "Max Requests Per Minute: " << config->get("ServiceLimits", "max_requests_per_minute") << std::endl;
        std::cout << "Max Memory Usage (MB): " << config->get("ServiceLimits", "max_memory_usage") << std::endl;

        // Access the ordered section 'DatabaseTable'
        std::cout << "\nDatabase Table Fields:" << std::endl;
        auto& tableFields = config->getOrderedSection("DatabaseTable");
        for (const auto& field : tableFields) {
            std::cout << field.first << " : " << field.second << std::endl;
        }

        // Access the list section 'AllowedServers'
        std::cout << "\nAllowed Servers:" << std::endl;
        auto allowedServers = config->getList("AllowedServers");
        for (const auto& server : allowedServers) {
            std::cout << server << std::endl;
        }

        // Access settings from the 'AdvancedOptions' section
        std::cout << "\nAdvanced Options:" << std::endl;
        std::cout << "Option X: " << config->get("AdvancedOptions", "option_x") << std::endl;
        std::cout << "Option Y: " << config->get("AdvancedOptions", "option_y") << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}