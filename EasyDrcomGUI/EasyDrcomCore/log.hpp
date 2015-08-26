/**
 * Copyright (C) 2014, 2015 Shindo
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

#ifndef __INCLUDE_LOG__
#define __INCLUDE_LOG__

std::string log_now();
extern std::ofstream log_stream;

#define LOG {                                                        \
    std::stringstream for_log_use_stream;                            \
    for_log_use_stream

#define PRINT_INFO                                                   \
    log_stream << log_now() << " " << for_log_use_stream.str();      \
    std::cout << for_log_use_stream.str();
#define PRINT_ERR                                                    \
    log_stream << log_now() << " " << for_log_use_stream.str();      \
    std::cout << for_log_use_stream.str();
#define PRINT_DBG                                                    \
    log_stream << log_now() << " " << for_log_use_stream.str();      \
    std::cout << for_log_use_stream.str();
#define PRINT_APPEND                                                 \
    log_stream << " " << for_log_use_stream.str();                   \
    std::cout << for_log_use_stream.str();

#define LOG_INFO(section, info)                                      \
    LOG << "[" << section << " Info] " << info; PRINT_INFO }
#define LOG_ERR(section, err)                                        \
    LOG << "[" << section << " Error] " << err; PRINT_ERR }
#ifdef EASYDRCOM_DEBUG
    #define LOG_DBG(section, db)                                     \
        LOG << "[" << section << " Debug] " << db; PRINT_DBG }
#else
    #define LOG_DBG(section, db)
#endif
#define LOG_APPEND(info)                                             \
    LOG << info; PRINT_APPEND }

#define U31_LOG_INFO(info)       LOG_INFO("U31", info)
#define U31_LOG_ERR(err)         LOG_ERR("U31", err)
#define U31_LOG_DBG(db)          LOG_DBG("U31", db)

#define U62_LOG_INFO(info)       LOG_INFO("U62", info)
#define U62_LOG_ERR(err)         LOG_ERR("U62", err)
#define U62_LOG_DBG(db)          LOG_DBG("U62", db)

#define EAP_LOG_INFO(info)  LOG_INFO("EAP", info)
#define EAP_LOG_ERR(err)    LOG_ERR("EAP", err)
#define EAP_LOG_DBG(db)     LOG_DBG("EAP", db)

#define SYS_LOG_INFO(info)  LOG_INFO("EasyDrcom", info)
#define SYS_LOG_ERR(err)    LOG_ERR("EasyDrcom", err)
#define SYS_LOG_DBG(db)     LOG_DBG("EasyDrcom", db)

#endif // __INCLUDE_LOG__
