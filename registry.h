/******************************************************************************
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
******************************************************************************/

#ifndef REGISTRY_H_
#define REGISTRY_H_

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

bool RegLoad(const char *file_name);
void RegSave(const char *file_name);

void RegSetString(const char *name, const char *value);
void RegSetInt32(const char *name, int value);
void RegSetFloat32(const char *name, float value);
void RegSetFloat64(const char *name, double value);

std::string RegGetString(const char *name, const char *default_value);
int         RegGetInt32(const char *name, int default_value);
float       RegGetFloat32(const char *name, float default_value);
double      RegGetFloat64(const char *name, double default_value);


#ifdef REGISTRY_IMPLEMENTATION

namespace {
std::map<std::string, std::string> kv_pairs;
std::mutex                         registry_mutex;

void TrimString(std::string &s, const char *t = " \t\n\r\f\v") {
    s.erase(0, s.find_first_not_of(t));
    s.erase(s.find_last_not_of(t) + 1);
}

void SplitString(const std::string &s, char delim, std::vector<std::string> &result) {
    std::stringstream ss(s);
    std::string       item;
    while (std::getline(ss, item, delim)) {
        if (item.length()) {
            result.push_back(item);
        }
    }
}

inline bool FileExists(const char *name) {
    std::ifstream f(name);
    return f.good();
}

void sys_error(const char *error) {
    MessageBoxA(0, error, "Error", MB_OK);
    abort();
}

void RemoveComments(std::string &line) {
    auto pos = line.find_last_of(';');
    if (pos != std::string::npos) {
        line.erase(pos, line.length() - pos);
    }
}

} // namespace

bool RegLoad(const char *file_name) {
    std::scoped_lock registry_lock(registry_mutex);

    if (FileExists(file_name)) {
        std::ifstream in("config.txt");
        std::string   line;

        kv_pairs.clear();

        while (std::getline(in, line)) {
            std::vector<std::string> tokens;

            RemoveComments(line);
            TrimString(line);
            SplitString(line, ' ', tokens);

            if (tokens.size() > 1) {
                std::string value;

                for (int i = 1; i < tokens.size(); i++) {
                    value += tokens[i];
                }

                kv_pairs[tokens[0]] = value;
            }
        }

        return true;
    }

    return false;
}

void RegSave(const char *file_name) {
    std::scoped_lock registry_lock(registry_mutex);

    for (auto &pair : kv_pairs) {
        std::ofstream f;

        f.open(file_name);
        f << pair.first << " " << pair.second << "\n";
    }
}

void RegSetString(const char *name, const char *value) {
    std::scoped_lock registry_lock(registry_mutex);

    kv_pairs[name] = value;
}

void RegSetInt32(const char *name, int value) {
    std::scoped_lock registry_lock(registry_mutex);

    kv_pairs[name] = std::to_string(value);
}

void RegSetFloat32(const char *name, float value) {
    std::scoped_lock registry_lock(registry_mutex);

    kv_pairs[name] = std::to_string(value);
}

void RegSetFloat64(const char *name, double value) {
    std::scoped_lock registry_lock(registry_mutex);

    kv_pairs[name] = std::to_string(value);
}

std::string RegGetString(const char *name, const char *default_value) {
    std::scoped_lock registry_lock(registry_mutex);

    auto value = kv_pairs.find(name);
    if (value != kv_pairs.end()) {
        return value->second;
    }
    return default_value;
}

int RegGetInt32(const char *name, int default_value) {
    std::scoped_lock registry_lock(registry_mutex);

    auto value = kv_pairs.find(name);
    if (value != kv_pairs.end()) {
        int ret = atoi(value->second.c_str());
        return ret;
    }
    return default_value;
}

float RegGetFloat32(const char *name, float default_value) {
    std::scoped_lock registry_lock(registry_mutex);

    auto value = kv_pairs.find(name);
    if (value != kv_pairs.end()) {
        float ret = std::stof(value->second.c_str());
        return ret;
    }
    return default_value;
}

double RegGetFloat64(const char *name, double default_value) {
    std::scoped_lock registry_lock(registry_mutex);

    auto value = kv_pairs.find(name);
    if (value != kv_pairs.end()) {
        double ret = std::stod(value->second.c_str());
        return ret;
    }
    return default_value;
}

#endif

#endif
