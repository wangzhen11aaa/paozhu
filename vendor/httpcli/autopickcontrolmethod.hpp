/**
 *  @copyright 2023, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file autopickcontrolmethod.hpp
 *  @date 2023-01-05
 *
 *  controller cpp file auto pick method to reg file
 *
 *
 */

#ifndef PROJECT_AUTOPICKCONTROLMETHOD_HPP
#define PROJECT_AUTOPICKCONTROLMETHOD_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <array>
#include <map>
#include <vector>
#include <cstring>
#include <string_view>
namespace http
{
namespace fs = std::filesystem;
struct reg_autoitem
{
    std::string pre;
    std::string func;
    std::string urlpath;
    std::string domain;
    std::string func_type;
    std::string filename;
    bool is_co = false;
};
struct file_regitem
{
    std::string filename;
    std::string filetime;
    std::string filehash;
};

bool str_casecmp_pre(std::string_view str1, std::string_view str2, unsigned int length)
{
    if (length == 0)
    {
        length = str2.size();
    }

    if (str1.size() < str2.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < length; i++)
    {
        if (i < str2.size())
        {
            if (str1[i] == str2[i])
            {
                continue;
            }
            else
            {
                if (str1[i] < 91 && str1[i] > 64)
                {
                    if ((str1[i] + 32) == str2[i])
                    {
                        continue;
                    }
                }
                else if (str2[i] < 91 && str2[i] > 64)
                {
                    if (str1[i] == (str2[i] + 32))
                    {
                        continue;
                    }
                }
                return false;
            }
        }
        else
        {
            return true;
        }
    }
    return true;
}

class pickcontrol
{
  public:
    void savecacheinfo(const std::string &methodpathfile, const std::map<std::string, struct file_regitem> &info_list)
    {

        std::unique_ptr<std::FILE, int (*)(std::FILE *)> f(fopen(methodpathfile.c_str(), "wb"), std::fclose);
        if (f == nullptr)
        {
            return;
        }
        std::string c_content;

        for (const auto &[first, second] : info_list)
        {
            c_content.append(second.filename);
            c_content.push_back(',');
            c_content.append(second.filetime);
            c_content.push_back(',');
            c_content.append(second.filehash);
            c_content.push_back(0x0A);
        }
        fwrite(&c_content[0], 1, c_content.size(), f.get());
    }
    std::map<std::string, struct file_regitem> loadcacheinfo(const std::string &methodpathfile)
    {
        std::map<std::string, struct file_regitem> reginfo_temp;
        struct file_regitem str_temp;
        std::string c_content;
        std::unique_ptr<std::FILE, int (*)(std::FILE *)> f(fopen(methodpathfile.c_str(), "rb"), std::fclose);
        if (f == nullptr)
        {
            return reginfo_temp;
        }
        fseek(f.get(), 0, SEEK_END);
        unsigned int file_size = ftell(f.get());
        fseek(f.get(), 0, SEEK_SET);
        c_content.resize(file_size);
        file_size = fread(&c_content[0], 1, file_size, f.get());
        c_content.resize(file_size);

        std::string tempstr;
        unsigned char foffset = 0;
        for (unsigned int i = 0; i < file_size; i++)
        {
            if (c_content[i] == 0x0A)
            {
                str_temp.filehash               = tempstr;
                reginfo_temp[str_temp.filename] = str_temp;

                str_temp.filename.clear();
                str_temp.filetime.clear();
                str_temp.filehash.clear();
                foffset = 0;
                tempstr.clear();
                continue;
            }
            if (c_content[i] == ',')
            {
                if (foffset == 0)
                {
                    str_temp.filename = tempstr;
                    tempstr.clear();
                    foffset = 1;
                }
                else if (foffset == 1)
                {
                    str_temp.filetime = tempstr;
                    tempstr.clear();
                    foffset = 2;
                }
                else
                {
                    str_temp.filehash = tempstr;
                    tempstr.clear();
                    foffset = 0;
                }
                continue;
            }
            if (c_content[i] == 0x20 || c_content[i] == '\t')
            {
                continue;
            }
            tempstr.push_back(c_content[i]);
        }
        if (tempstr.size() > 0)
        {
            if (foffset == 0)
            {
                str_temp.filename = tempstr;
                tempstr.clear();
                foffset = 1;
            }
            else if (foffset == 1)
            {
                str_temp.filetime = tempstr;
                tempstr.clear();
                foffset = 2;
            }
            else
            {
                str_temp.filehash = tempstr;
                tempstr.clear();
                foffset = 0;
            }
            reginfo_temp[str_temp.filename] = str_temp;
        }
        return reginfo_temp;
    }
    std::vector<std::string> listpath(const std::string &methodpath)
    {
        fs::path tagetpath = methodpath;
        std::vector<std::string> temp;
        if (fs::exists(tagetpath) && fs::is_directory(tagetpath))
        {

            for (const auto &entry : fs::directory_iterator(tagetpath))
            {
                auto filename = entry.path().filename().string();
                if (fs::is_regular_file(entry.status()))
                {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".cpp")
                    {
                        temp.emplace_back(filename.substr(0, filename.size() - 4));
                    }
                }
                else if (fs::is_directory(entry.status()))
                {
                    std::string ext;
                    if (methodpath.back() == '/')
                    {

                        ext.append(methodpath);
                        ext.append(filename);
                    }
                    else
                    {
                        ext.append(methodpath);
                        ext.append("/");
                        ext.append(filename);
                    }
                    std::vector<std::string> directory_temp = listpath(ext);
                    if (directory_temp.size() > 0)
                    {
                        for (unsigned int j = 0; j < directory_temp.size(); j++)
                        {
                            ext.clear();
                            ext.append(filename);
                            ext.append("/");
                            ext.append(directory_temp[j]);
                            temp.emplace_back(ext);
                        }
                    }
                }
            }
        }

        return temp;
    }
    std::vector<struct reg_autoitem> pickfile(const std::string &methodpathfile)
    {

        std::vector<struct reg_autoitem> temp;

        std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(methodpathfile.c_str(), "rb"), std::fclose);

        if (!fp.get())
        {
            return temp;
        }

        std::string filecontent;

        fseek(fp.get(), 0, SEEK_END);
        unsigned long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);

        filecontent.resize(file_size);
        file_size = fread(&filecontent[0], 1, file_size, fp.get());
        filecontent.resize(file_size);

        bool isbegin = true;
        for (unsigned int i = 0; i < filecontent.size(); i++)
        {
            if (isbegin == false)
            {
                if (filecontent[i] == 0x0A)
                {
                    isbegin = true;
                }
                continue;
            }

            if (filecontent[i] > 0x2F)
            {
                isbegin = false;
                continue;
            }
            bool is_match            = false;
            unsigned int offset_temp = 0;
            if (str_casecmp_pre(&filecontent[i], "//@urlpath(", 0))
            {
                is_match    = true;
                offset_temp = 11;
            }
            else if (str_casecmp_pre(&filecontent[i], "// @urlpath(", 0))
            {
                is_match    = true;
                offset_temp = 12;
            }

            if (is_match)
            {
                std::string prename;
                std::string tempname;
                unsigned int j = i + offset_temp;
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == ')' || filecontent[j] == 0x0A)
                    {
                        // j++;
                        break;
                    }
                    if (filecontent[j] == ',')
                    {
                        prename = tempname;
                        tempname.clear();
                        continue;
                    }
                    if (filecontent[j] == '"')
                    {
                        continue;
                    }
                    if (filecontent[j] == '\t')
                    {
                        continue;
                    }
                    if (filecontent[j] == 0x20)
                    {
                        continue;
                    }
                    if (filecontent[j] == 0x0d)
                    {
                        continue;
                    }
                    tempname.push_back(filecontent[j]);
                }

                i = j;
                std::string linestr, func_pre_type;
                if (tempname.empty())
                {
                    isbegin = false;
                    continue;
                }
                //process remaining
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == 0x0A)
                    {
                        j++;
                        break;
                    }
                }
                for (; j < filecontent.size(); j++)
                {
                    if (filecontent[j] == 0x0A || filecontent[j] == '(')
                    {
                        if (linestr.size() < 2)
                        {
                            linestr.clear();
                            if (filecontent[j] == 0x0A)
                            {
                                continue;
                            }

                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == 0x0A)
                                {
                                    break;
                                }
                            }
                            continue;
                        }
                        if (filecontent[j] == '(')
                        {
                            break;
                        }
                        // func type maybe alone line
                        func_pre_type = linestr;
                        linestr.clear();
                        continue;
                    }
                    else if (filecontent[j] == '/')
                    {
                        // is ship annotation
                        linestr.clear();
                        unsigned int nnn = j + 1;
                        if (nnn < filecontent.size() && filecontent[nnn] == '*')
                        {
                            j += 2;
                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == '*')
                                {
                                    nnn = j + 1;
                                    if (nnn < filecontent.size() && filecontent[nnn] == '/')
                                    {
                                        j++;
                                        break;
                                    }
                                }
                            }
                            continue;
                        }
                        else
                        {
                            for (; j < filecontent.size(); j++)
                            {
                                if (filecontent[j] == 0x0A)
                                {
                                    break;
                                }
                            }
                        }
                        continue;
                    }

                    if (filecontent[j] == 0x20 || filecontent[j] == '\t')
                    {
                        unsigned int jj = j + 1;
                        for (; jj < filecontent.size(); jj++)
                        {
                            if (filecontent[jj] == 0x20 || filecontent[jj] == '\t')
                            {
                                j++;
                                continue;
                            }
                            break;
                        }
                        if (jj < filecontent.size() && filecontent[jj] == '(')
                        {
                            break;
                        }
                        if (jj < filecontent.size() && filecontent[jj] == 0x0A)
                        {
                            continue;
                        }

                        if (func_pre_type.empty() && linestr.size() > 5)
                        {
                            func_pre_type = linestr;
                        }
                        linestr.clear();
                        continue;
                    }
                    if (filecontent[j] == 0x0d)
                    {
                        continue;
                    }
                    linestr.push_back(filecontent[j]);
                }

                if (linestr.size() < 2)
                {
                    //again process
                    for (; j < filecontent.size(); j++)
                    {
                        if (filecontent[j] == 0x0A)
                        {
                            j++;
                            break;
                        }
                    }
                    for (; j < filecontent.size(); j++)
                    {
                        if (filecontent[j] == 0x0A || filecontent[j] == '(')
                        {
                            // j++;
                            break;
                        }

                        if (filecontent[j] == 0x20 || filecontent[j] == '\t')
                        {
                            //cut space
                            unsigned int jj = j + 1;
                            for (; jj < filecontent.size(); jj++)
                            {
                                if (filecontent[jj] == 0x20 || filecontent[jj] == '\t')
                                {
                                    j++;
                                    continue;
                                }
                                break;
                            }
                            if (jj < filecontent.size() && filecontent[jj] == '(')
                            {
                                break;
                            }
                            if (func_pre_type.empty() && linestr.size() > 5)
                            {
                                func_pre_type = linestr;
                            }
                            linestr.clear();
                            continue;
                        }
                        linestr.push_back(filecontent[j]);
                    }
                }

                i       = j;
                isbegin = false;
                struct reg_autoitem reg_temp;
                reg_temp.pre     = prename;
                reg_temp.urlpath = tempname;
                //process left right space
                prename.clear();
                for (unsigned int iii = 0; iii < linestr.size(); iii++)
                {
                    if (linestr[iii] == ' ' || linestr[iii] == '\t' || linestr[iii] == 0x0D || linestr[iii] == 0x0A)
                    {
                        continue;
                    }
                    prename.push_back(linestr[iii]);
                }
                reg_temp.func = prename;

                //process left right space
                prename.clear();
                for (unsigned int iii = 0; iii < func_pre_type.size(); iii++)
                {
                    if (func_pre_type[iii] == ' ' || func_pre_type[iii] == '\t' || func_pre_type[iii] == 0x0D || func_pre_type[iii] == 0x0A)
                    {
                        continue;
                    }
                    prename.push_back(func_pre_type[iii]);
                }
                reg_temp.func_type = prename;
                if (prename == "asio::awaitable<std::string>")
                {
                    reg_temp.is_co = true;
                }
                else
                {
                    reg_temp.is_co = false;
                }

                reg_temp.filename = methodpathfile;
                temp.emplace_back(reg_temp);
                continue;
            }
        }

        return temp;
    }
    void createhfile(const std::string &filename, const std::vector<struct reg_autoitem> &methodpathfile)
    {
        std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(filename.c_str(), "wb"), std::fclose);

        if (!fp.get())
        {
            return;
        }

        std::string header_content = R"(
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
)";
        for (unsigned int i = 0; i < methodpathfile.size(); i++)
        {
            if (methodpathfile[i].func_type == "std::string")
            {
                header_content.append("\tstd::string ");
                header_content.append(methodpathfile[i].func);
                header_content.append("(std::shared_ptr<httppeer> peer);\r\n");
            }
            else if (methodpathfile[i].func_type == "asio::awaitable<std::string>")
            {
                header_content.append("\tasio::awaitable<std::string> ");
                header_content.append(methodpathfile[i].func);
                header_content.append("(std::shared_ptr<httppeer> peer);\r\n");
            }
        }
        header_content.append("}\r\n");

        fwrite(&header_content[0], 1, header_content.size(), fp.get());
    }
    // public:
    //     std::map<std::string, time_t> fileslist;
    //     std::map<std::string, time_t> sofileslist;
};
}// namespace http
#endif// PROJECT_AUTOPICKMEMTHOD_HPP