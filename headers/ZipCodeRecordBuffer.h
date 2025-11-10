#ifndef ZipCodeRecordBuffer_H
#define ZipCodeRecordBuffer_H

#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cctype>
#include <vector>

const int ZIP_CODE_LENGTH = 5;
const int PLACE_NAME_LENGTH = 50;
const int STATE_LENGTH = 2;
const int COUNTY_LENGTH = 50;
const int LAT_LONG_LENGTH = 10;

class ZipCodeRecordBuffer {
public:
    ZipCodeRecordBuffer() {
        for (int i = 0; i < 6; ++i) m_fields[i] = "";
    }

    // Reads until a valid data record is found or EOF; returns true when a valid record is parsed
    bool ReadRecord(std::istream& file) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            // parse CSV fields (simple split by comma) - handle up to 7 columns
            std::vector<std::string> fields;
            std::istringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ',')) {
                trim(token);
                // remove surrounding quotes
                if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
                    token = token.substr(1, token.size() - 2);
                    trim(token);
                }
                fields.push_back(token);
            }

            // If not 6 or 7 fields, skip line
            if (fields.size() < 6) continue;
            if (fields.size() > 7) {
             // keep first 6 tokens (or merge extras into the last token)
            fields.resize(6);
            }
            bool hasRecordLength = false;
            // Detect optional RecordLength field: treat as numeric integer (all digits)
            if (fields.size() == 7) {
                const std::string &f0 = fields[0];
                bool allDigits = !f0.empty() && std::all_of(f0.begin(), f0.end(), [](unsigned char c){
                    return std::isdigit(c);
                });
                if (allDigits) hasRecordLength = true;
                else {
                    // maybe header with "RecordLength" text: skip header
                    std::string up0 = f0;
                    std::transform(up0.begin(), up0.end(), up0.begin(), [](unsigned char c){ return std::toupper(c); });
                    if (up0.find("RECORD") != std::string::npos) continue;
                    // otherwise, accept as 7th field but treat as not record length (rare)
                }
            }

            // Determine indices for fields: if hasRecordLength, zip is fields[1], else fields[0]
            int zipId = hasRecordLength ? 1 : 0;
            int placeId = zipId + 1;
            int stateId = zipId + 2;
            int countyId = zipId + 3;
            int latId = zipId + 4;
            int lonId = zipId + 5;

            // Basic header detection: if zip field contains "ZIP" or "POSTAL", skip
            std::string zipCandidate = fields[zipId];
            std::string upZip = zipCandidate;
            std::transform(upZip.begin(), upZip.end(), upZip.begin(), [](unsigned char c){ return std::toupper(c); });
            if (upZip.find("ZIP") != std::string::npos || upZip.find("POSTAL") != std::string::npos) {
                continue;
            }

            // Now map into m_fields (we always keep 6 logical fields)
            m_fields[0] = truncateTo(fields[zipId], ZIP_CODE_LENGTH);
            m_fields[1] = truncateTo(fields[placeId], PLACE_NAME_LENGTH);
            m_fields[2] = truncateTo(fields[stateId], STATE_LENGTH);
            m_fields[3] = truncateTo(fields[countyId], COUNTY_LENGTH);
            std::string latStr = truncateTo(fields[latId], LAT_LONG_LENGTH);
            std::string lonStr = truncateTo(fields[lonId], LAT_LONG_LENGTH);

            // Try converting lat/lon
            try {
                // std::stod tolerates leading/trailing spaces
                latitude = std::stod(latStr);
                longitude = std::stod(lonStr);
            } catch (...) {
                // malformed numeric fields -> skip line
                continue;
            }

            // success
            return true;
        }
        // EOF reached without a valid data record
        return false;
    }

    std::string getZipCode() const { return m_fields[0]; }
    std::string getPlaceName() const { return m_fields[1]; }
    std::string getState() const { return m_fields[2]; }
    std::string getCounty() const { return m_fields[3]; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }

private:
    std::string m_fields[6];
    double latitude = std::numeric_limits<double>::quiet_NaN();
    double longitude = std::numeric_limits<double>::quiet_NaN();

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
