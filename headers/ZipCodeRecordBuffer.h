#ifndef ZipCodeRecordBuffer_H
#define ZipCodeRecordBuffer_H

#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cctype>
#include <vector>
#include <iostream>
#include <iomanip>

// (Your existing const lengths)
const int ZIP_CODE_LENGTH = 5;
const int PLACE_NAME_LENGTH = 50;
const int STATE_LENGTH = 2;
const int COUNTY_LENGTH = 50;
const int LAT_LONG_LENGTH = 10;

class ZipCodeRecordBuffer {
public:
    ZipCodeRecordBuffer() {
        for (int i = 0; i < 6; ++i) m_fields[i] = "";
        latitude = 0.0;
        longitude = 0.0;
    }

    /**
     * @brief Unpacks a comma-separated string (a single record) into this object's fields.
     * @param recordString The comma-separated string (e.g., "56301,St Cloud,MN,...").
     * @return True on success, false on failure.
     * @note This directly addresses Task 5: "The record buffer unpacks fields".
     */
    bool unpack(const std::string& recordString) {
        std::istringstream ss(recordString);
        std::string token;
        
        try {
            std::getline(ss, m_fields[0], ','); // Zip
            std::getline(ss, m_fields[1], ','); // Place
            std::getline(ss, m_fields[2], ','); // State
            std::getline(ss, m_fields[3], ','); // County
            
            std::getline(ss, token, ','); // Lat
            latitude = std::stod(token);
            
            std::getline(ss, token, ','); // Lon
            longitude = std::stod(token);
            
            return true;
        } catch (const std::exception& e) {
            // Handle error (e.g., bad stod conversion)
            return false;
        }
    }

    /**
     * @brief Packs this object's fields into a single comma-separated string.
     * @return The comma-separated string.
     * @note This is the inverse of unpack().
     */
    std::string pack() const {
        std::ostringstream ss;
        ss << m_fields[0] << ','    // Zip
           << m_fields[1] << ','    // Place name
           << m_fields[2] << ','    // State
           << m_fields[3] << ','    // County
           << latitude << ','       // Latitude
           << longitude;            // Longitude
        return ss.str();
    }

    /**
     * @brief Reads a record from a CSV file stream.
     * @note This is your original ReadRecord, still useful for the initial file creation.
     */
    bool ReadRecord(std::istream& file) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            // (Your existing CSV parsing logic)
            std::vector<std::string> fields;
            std::istringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ',')) {
                trim(token);
                if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
                    token = token.substr(1, token.size() - 2);
                    trim(token);
                }
                fields.push_back(token);
            }

            if (fields.size() < 6) continue;
            if (fields.size() > 7) fields.resize(6);
            
            bool hasRecordLength = false;
            if (fields.size() == 7) {
                 // (Your logic for detecting record length or header)
                const std::string &f0 = fields[0];
                bool allDigits = !f0.empty() && std::all_of(f0.begin(), f0.end(), [](unsigned char c){
                    return std::isdigit(c);
                });
                if (allDigits) hasRecordLength = true;
                else {
                    std::string up0 = f0;
                    std::transform(up0.begin(), up0.end(), up0.begin(), [](unsigned char c){ return std::toupper(c); });
                    if (up0.find("RECORD") != std::string::npos) continue;
                }
            }

            int zipId = hasRecordLength ? 1 : 0;
            int placeId = zipId + 1;
            int stateId = zipId + 2;
            int countyId = zipId + 3;
            int latId = zipId + 4;
            int lonId = zipId + 5;

            std::string zipCandidate = fields[zipId];
            std::string upZip = zipCandidate;
            std::transform(upZip.begin(), upZip.end(), upZip.begin(), [](unsigned char c){ return std::toupper(c); });
            if (upZip.find("ZIP") != std::string::npos || upZip.find("POSTAL") != std::string::npos) {
                continue;
            }
            if (zipCandidate.empty() || !std::all_of(zipCandidate.begin(), zipCandidate.end(), ::isdigit)) {
                continue; // Skip if zip is not purely numeric
            }

            m_fields[0] = truncateTo(fields[zipId], ZIP_CODE_LENGTH);
            m_fields[1] = truncateTo(fields[placeId], PLACE_NAME_LENGTH);
            m_fields[2] = truncateTo(fields[stateId], STATE_LENGTH);
            m_fields[3] = truncateTo(fields[countyId], COUNTY_LENGTH);
            std::string latStr = truncateTo(fields[latId], LAT_LONG_LENGTH);
            std::string lonStr = truncateTo(fields[lonId], LAT_LONG_LENGTH);

            try {
                latitude = std::stod(latStr);
                longitude = std::stod(lonStr);
            } catch (...) {
                continue;
            }
            return true;
        }
        return false;
    }

    void print() const {
        std::cout << "---------------------------------------------\n"
                  << "ZIP Code: " << m_fields[0] << "\n"
                  << "Place Name: " << m_fields[1] << "\n"
                  << "State: " << m_fields[2] << "\n"
                  << "County: " << m_fields[3] << "\n"
                  << "Latitude: " << latitude << "\n"
                  << "Longitude: " << longitude << "\n";
    }
    
    std::string getZipCode() const { return m_fields[0]; }
    std::string getPlaceName() const { return m_fields[1]; }
    std::string getState() const { return m_fields[2]; }
    std::string getCounty() const { return m_fields[3]; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }

private:
    std::string m_fields[6];
    double latitude;
    double longitude;

    static inline void trim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    static inline std::string truncateTo(const std::string &s, size_t maxLen) {
        if (s.length() <= maxLen) return s;
        return s.substr(0, maxLen);
    }
};

#endif // ZipCodeRecordBuffer_H