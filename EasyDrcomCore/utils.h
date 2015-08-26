/**
 * Copyright (C) 2015 Shindo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EasyDrcomGUI_utils_h
#define EasyDrcomGUI_utils_h

#define EASYDRCOM_DEBUG

std::vector<uint8_t> get_md5_digest(std::vector<uint8_t>& data);
std::string hex_to_str(uint8_t *hex, size_t len, char separator);
void hexdump(std::vector<uint8_t> hex);
std::vector<std::string> split_string(std::string src, char delimiter = ' ', bool append_last = true);
std::vector<uint8_t> str_ip_to_vec(std::string ip);
std::vector<uint8_t> str_mac_to_vec(std::string mac);
std::vector<uint8_t> str_to_vec(std::string str);

#endif
