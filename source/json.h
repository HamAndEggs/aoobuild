/*
   Copyright (C) 2017, Richard e Collins.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
   
#ifndef __JSON_H__
#define __JSON_H__

#include <fstream>
#include <sstream>

#include <string>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

namespace appbuild{
//////////////////////////////////////////////////////////////////////////
// Some rapid json helpers.
inline bool SaveJson(const std::string& pFilename,rapidjson::Document& pJson)
{
   std::ofstream file(pFilename);
   if( file )
   {
      rapidjson::OStreamWrapper osw(file);
      rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
      return pJson.Accept(writer);
   }
   return false;
}

inline bool ReadJson(const std::string& pFilename,rapidjson::Document& pJson)
{
   std::ifstream jsonFile(pFilename);
   if( jsonFile.is_open() )
   {
      std::stringstream jsonStream;
      jsonStream << jsonFile.rdbuf();// Read the whole file in...

      pJson.Parse(jsonStream.str().c_str());

      // Have to invert result as I want tru if it worked, false if it failed.
      return pJson.HasParseError() == false;
   }
   return false;
}

inline void PushBack(rapidjson::Value& pArray,const std::string& Value,rapidjson::Document::AllocatorType& pAlloc)
{
   pArray.PushBack(rapidjson::Value(Value,pAlloc),pAlloc);
}

// Not using overloads as I want to keep to the rapid json naming. Also means I can build them all with some macro magic.
#define UTIL_GET_WITH_DEFAULT_FUNCTIONS            \
   ADD_FUNCTION(GetBool,IsBool,bool)               \
   ADD_FUNCTION(GetInt,IsInt,int)                  \
   ADD_FUNCTION(GetUint,IsUint,unsigned)           \
   ADD_FUNCTION(GetInt64,IsInt64,int64_t)          \
   ADD_FUNCTION(GetUint64,IsUint64,uint64_t)       \
   ADD_FUNCTION(GetFloat,IsFloat,float)            \
   ADD_FUNCTION(GetDouble,IsDouble,double)         \
   ADD_FUNCTION(GetString,IsString,std::string)    \

#define ADD_FUNCTION(__GET_NAME__,__CHECK_NAME__,__TYPE__)                                                  \
   inline __TYPE__ __GET_NAME__(const rapidjson::Value& pJson,const std::string& pKey,__TYPE__ pDefault)    \
   {                                                                                                        \
      if( pJson.HasMember(pKey) )                                                                           \
      {                                                                                                     \
         assert( pJson[pKey].__CHECK_NAME__() );                                                            \
         if( pJson[pKey].__CHECK_NAME__() )                                                                 \
            return pJson[pKey].__GET_NAME__();                                                              \
      }                                                                                                     \
      return pDefault;                                                                                      \
   }

// Build all out util get functions that allow you to define a default if the item is not in the json stream.
UTIL_GET_WITH_DEFAULT_FUNCTIONS

#undef ADD_FUNCTION

//////////////////////////////////////////////////////////////////////////
};//namespace appbuild


#endif //__JSON_H__