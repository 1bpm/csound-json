/*
    opcodes.cpp
    Copyright (C) 2022 Richard Knight


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
 */
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
#include <plugin.h>
#include "handling.h"

#define ARGT static constexpr char const


const char* badHandle = "cannot obtain data from handle";
const char* deadHandle = "object has been destroyed";
const char* handleName = "jsonsession";

struct JSONSession {
    jsoncons::json data;
    bool active;
};

/*
    Initialise an array and return STRINDAT pointer in case it is required
 */
STRINGDAT* arrayInit(csnd::Csound* csound, ARRAYDAT* array, int rows, int cols) {
    int totalResults = rows * cols;
    size_t totalAllocated;
    
    //if (array->data == NULL) {
        array->sizes = (int32_t*) csound->calloc(sizeof(int32_t) * 2);
        array->sizes[0] = rows;
        if (cols != 1) {
            array->sizes[1] = cols;
        }
        array->dimensions = cols;
        CS_VARIABLE *var = array->arrayType->createVariable(csound->get_csound(), NULL);
        array->arrayMemberSize = var->memBlockSize;
        totalAllocated = array->arrayMemberSize * totalResults;
        array->data = (MYFLT*) csound->calloc(totalAllocated);
    // } else
    if ((totalAllocated = array->arrayMemberSize * totalResults) > array->allocated) {
        array->data = (MYFLT*) csound->realloc(array->data, totalAllocated);
        memset((char*)(array->data)+array->allocated, '\0', totalAllocated - array->allocated);
        array->allocated = totalAllocated;
    }
    
    // convenience return to be used if it is a string array
    return (STRINGDAT*) array->data;
}


/*
 Insert a string to an array
 */
void insertArrayStringItem(csnd::Csound* csound, STRINGDAT* strings, int index, char* item) {
    strings[index].size = strlen(item) + 1;
    if (strings[index].data != NULL) {
        csound->free(strings[index].data);
    }
    strings[index].data = csound->strdup(item);
}


/*
 Convert JSON array to Csound array either as string or numeric
 */
void jsonArrayToCSArray(csnd::Csound* csound, jsoncons::json* jdatap, ARRAYDAT* array, bool asString) {
    jsoncons::json jdata = *jdatap;
    std::vector<std::string> vals = jdata.as<std::vector<std::string>>();
    
    STRINGDAT* strings = arrayInit(csound, array, vals.size(), 1);
    
    char* value;
    for (std::size_t index = 0; index < vals.size(); index++) {
        value = (char*) vals[index].c_str();
        if (asString) {
            insertArrayStringItem(csound, strings, index, value);
        } else {
            array->data[index] = (MYFLT) atof(value);
        }        
    }
}


/*
 Get the JSON type of a session object
 */
int getJsonType(JSONSession* jsonSession) {
    jsoncons::json j = jsonSession->data;
    int outtype = -1;
    if (j.is_null()) {
        outtype = 0;
    } else if (j.is_string()) {
        outtype = 1;
    } else if (j.is_number()) {
        outtype = 2;
    } else if (j.is_bool()) {
        outtype = 3;
    } else if (j.is_array()) {
        outtype = 4;
    } else if (j.is_object()) {
        outtype = 5;
    }
    return outtype;
}

// cs AppendOpcode mallocs struct so virtual functions cannot be used. 
// Macro workaround to fake struct derivation type model, just chuck it all in a macro...

#define _PLUGINSESSIONBASE(idInArgs)\
    JSONSession* jsonSession;\
    void getSession() {\
        if (!(jsonSession = getHandle<JSONSession>(csound, idInArgs[0], handleName))) {\
			throw std::runtime_error(badHandle);\
		}\
        if (!jsonSession->active) throw std::runtime_error(deadHandle);\
    }\
	void getSession(MYFLT handle, JSONSession** returnSession) {\
		if (!(*returnSession = getHandle<JSONSession>(csound, handle, handleName))) {\
			throw std::runtime_error(badHandle);\
		}\
        if (!jsonSession->active) throw std::runtime_error(deadHandle);\
	}

#define _PLUGINITBASE(votypes, vitypes, doGetSession) \
	ARGT* otypes = votypes;\
	ARGT* itypes = vitypes;\
	int init() {\
		try {\
			if (doGetSession) getSession();\
			irun();\
		} catch (const std::exception &ex) {\
			return csound->init_error(ex.what());\
		}\
		return OK;\
	}

// extension of above for k-rate opcodes
#define _PLUGINITBASEK(votypes, vitypes, doGetSession) \
    _PLUGINITBASE(votypes, vitypes, doGetSession)\
    void irun() {}\
    int kperf() {\
        try {\
            krun();\
        } catch (const std::exception &ex) {\
            return csound->perf_error(ex.what(), this);\
        }\
        return OK;\
    }

#define PLUGINSESSION \
    _PLUGINSESSIONBASE(inargs)

#define INPLUGSESSION \
    _PLUGINSESSIONBASE(args)

#define PLUGINIT(votypes, vitypes, doGetSession) \
	_PLUGINITBASE(votypes, vitypes, doGetSession)

#define INPLUGINIT(vitypes) \
	_PLUGINITBASE("", vitypes, true)

#define PLUGINITK(votypes, vitypes, doGetSession) \
	_PLUGINITBASEK(votypes, vitypes, doGetSession)

#define INPLUGINITK(vitypes) \
	_PLUGINITBASEK("", vitypes, true)
            
#define INPLUGCHILD(vitypes) \
    INPLUGINIT(vitypes)\
    void irun() { run(); }

#define INPLUGCHILDK(vitypes) \
    INPLUGINITK(vitypes)\
    void krun() { run(); }

#define PLUGINCHILD(votypes, vitypes, doGetSession) \
    PLUGINIT(votypes, vitypes, doGetSession)\
    void irun() { run(); }

#define PLUGINCHILDK(votypes, vitypes, doGetSession) \
    PLUGINITK(votypes, vitypes, doGetSession)\
    void krun() { run(); }



template <std::size_t N> 
struct inplug : csnd::InPlug<N> {
    using csnd::InPlug<N>::args;
    using csnd::InPlug<N>::csound;
    INPLUGSESSION
};

template <std::size_t N, std::size_t M> 
struct plugin : csnd::Plugin<N, M> {
    using csnd::Plugin<N, M>::inargs;
    using csnd::Plugin<N, M>::outargs;
    using csnd::Plugin<N, M>::csound;
    PLUGINSESSION
};


/*
 Parse JSON object from a provided string
 */
struct jsonloads : plugin<1, 1> {
	PLUGINIT("i", "S", false)
	void irun() {
        outargs[0] = createHandle<JSONSession>(csound, &jsonSession, handleName);
        jsonSession->active = true;
        jsonSession->data = jsoncons::json::parse(std::string(inargs.str_data(0).data));
	}
};


/*
 Initialise an empty JSON object
 */
struct jsoninit : plugin<1, 0> {
	PLUGINIT("i", "", false)
	void irun() {
        outargs[0] = createHandle<JSONSession>(csound, &jsonSession, handleName);
        jsonSession->active = true;
        jsonSession->data = jsoncons::json::parse("{}");
	}
};


/*
 Merge two JSON objects
 */
struct jsonmerge : inplug<3> {
	INPLUGINIT("iio")
	int irun() {        
        JSONSession* jsonSession2;
        getSession(args[1], &jsonSession2);
        if (args[2] == 1) { 
            jsonSession->data.merge_or_update(jsonSession2->data);
        } else {
            jsonSession->data.merge(jsonSession2->data);
        }

	}
};


/*
 Insert a JSON object to another JSON object with specified key
 */
struct jsoninsert : inplug<3> {
	INPLUGINIT("iSi")
	void irun() {
        JSONSession* jsonSession2;
        getSession(args[2], &jsonSession2);
        jsonSession->data.insert_or_assign(
            std::string(args.str_data(1).data),
            jsonSession2->data
        );
	}
};


/*
Insert an array of JSON objects to another JSON object with specified key        
*/      
struct jsoninsertArray : inplug<3> {
	INPLUGINIT("iSi[]")
	void irun() {      
        JSONSession* jsonSession2;
        ARRAYDAT* values = (ARRAYDAT*) args(2);
        std::vector<jsoncons::json> valuesVector;
        
        for (int i = 0; i < values->sizes[0]; i++) {
            getSession(values->data[i], &jsonSession2);
            valuesVector.push_back(jsonSession2->data);
        }
               
        jsonSession->data.insert_or_assign(
            std::string(args.str_data(1).data),
            valuesVector
        );
	}
};


/*
 Insert a string value to a JSON object with specified key
 */
struct jsoninsertvalStringBase : inplug<3> {
    void run() {
        jsonSession->data.insert_or_assign(
            std::string(args.str_data(1).data),
            std::string(args.str_data(2).data)
        );
	}
};
struct jsoninsertvalString : jsoninsertvalStringBase {
	INPLUGCHILD("iSS")
};
struct jsoninsertvalStringK : jsoninsertvalStringBase {
	INPLUGCHILDK("iSS")
};


/*
 Insert a numeric value to a JSON object with specified key
 */
struct jsoninsertvalNumericBase : inplug<3> {
    void run() {
        jsonSession->data.insert_or_assign(
            std::string(args.str_data(1).data),
            args[2]
        );  
    }
};
struct jsoninsertvalNumeric : jsoninsertvalNumericBase { 
    INPLUGCHILD("iSi") 
};
struct jsoninsertvalNumericK : jsoninsertvalNumericBase { 
    INPLUGCHILDK("iSk") 
};



/*
 Insert a numeric array to a JSON object with specified key
 */
struct jsoninsertvalNumericArrayBase : inplug<3> {
    void run() {
        ARRAYDAT* values = (ARRAYDAT*) args(2);
        std::vector<MYFLT> valuesVector(values->data, values->data + values->sizes[0]);
        jsonSession->data.insert_or_assign(
            std::string(args.str_data(1).data),
            valuesVector
        );
    }
};
struct jsoninsertvalNumericArray : jsoninsertvalNumericArrayBase { 
    INPLUGCHILD("iSi[]") 
};
struct jsoninsertvalNumericArrayK : jsoninsertvalNumericArrayBase { 
    INPLUGCHILDK("iSk[]") 
};



/*
 Insert a string array to a JSON object with specified key
 */
struct jsoninsertvalStringArrayBase : inplug<3> {	
	void run() { 
        ARRAYDAT* values = (ARRAYDAT*) args(2);
        STRINGDAT* strings = (STRINGDAT*) values->data;
        std::vector<std::string> valuesVector;
        for (int i = 0; i < values->sizes[0]; i++) {
            valuesVector.push_back(std::string(strings[i].data));
        }
        jsonSession->data.insert_or_assign(
            std::string(args.str_data(1).data),
            valuesVector
        );
	}
};
struct jsoninsertvalStringArray : jsoninsertvalStringArrayBase { 
    INPLUGCHILD("iSS[]") 
};
struct jsoninsertvalStringArrayK : jsoninsertvalStringArrayBase { 
    INPLUGCHILDK("iSS[]") 
};


/*
 Insert string key, string value pairs to a JSON object with specified key
 */
struct jsoninsertvalStringStringArrayBase : inplug<3> {
    void run() {
        ARRAYDAT* rawKeys = (ARRAYDAT*) args(1);
        STRINGDAT* keys = (STRINGDAT*) rawKeys->data;
        ARRAYDAT* rawValues = (ARRAYDAT*) args(2);
        STRINGDAT* values = (STRINGDAT*) rawValues->data;
        if (rawKeys->sizes[0] != rawValues->sizes[0]) {
            throw std::runtime_error("key and value arrays are not the same size");
        }
        for (int i = 0; i < rawKeys->sizes[0]; i++) {
            jsonSession->data.insert_or_assign(
                std::string(keys[i].data),
                std::string(values[i].data)
            );
        }
    }
};
struct jsoninsertvalStringStringArray : jsoninsertvalStringStringArrayBase {
    INPLUGCHILD("iS[]S[]")
};
struct jsoninsertvalStringStringArrayK : jsoninsertvalStringStringArrayBase {
    INPLUGCHILDK("iS[]S[]")
};
 

/*
 Insert string key, numeric value pairs to a JSON object with specified key
 */
struct jsoninsertvalStringNumericArrayBase : inplug<3> {
    void run() {
        ARRAYDAT* rawKeys = (ARRAYDAT*) args(1);
        STRINGDAT* keys = (STRINGDAT*) rawKeys->data;
        ARRAYDAT* rawValues = (ARRAYDAT*) args(2);
        if (rawKeys->sizes[0] != rawValues->sizes[0]) {
            throw std::runtime_error("key and value arrays are not the same size");
        }
        for (int i = 0; i < rawKeys->sizes[0]; i++) {
            jsonSession->data.insert_or_assign(
                std::string(keys[i].data),
                rawValues->data[i] // not like doubles?
            );
        }
    }
};
struct jsoninsertvalStringNumericArray : jsoninsertvalStringNumericArrayBase {
    INPLUGCHILD("iS[]i[]")
};
struct jsoninsertvalStringNumericArrayK : jsoninsertvalStringNumericArrayBase {
    INPLUGCHILDK("iS[]k[]")
};


/*
 Get JSON type as number
 */
struct jsontype : plugin<1, 1> {
	PLUGINIT("i", "i", true)
    void irun() {
        outargs[0] = (MYFLT) getJsonType(jsonSession);
    }
};

/*
 Get JSON type as string
 */
struct jsontypeString : plugin<1, 1> {
	PLUGINIT("S", "i", true)
    void irun() {
        STRINGDAT &sdoutput = outargs.str_data(0);
        int type = getJsonType(jsonSession);
        std::string output;
        switch (type) {
            case -1:
                output.assign("unknown");
                break;
            case 0:
                output.assign("null");
                break;
            case 1:
                output.assign("string");
                break;
            case 2:
                output.assign("number");
                break;
            case 3:
                output.assign("boolean");
                break;
            case 4:
                output.assign("array");
                break;
            case 5:
                output.assign("object");
                break;
        }
        sdoutput.size = output.length();
        sdoutput.data = csound->strdup((char*) output.c_str());
    }
};

/*
 Get the key names from an object
 */
struct jsonkeysBase : plugin<1, 1> {
    void run() {
        std::map<std::string, jsoncons::json> map = 
                jsonSession->data.as<std::map<std::string, jsoncons::json>>();
        ARRAYDAT* array = (ARRAYDAT*) outargs(0);
        STRINGDAT* strings = arrayInit(csound, array, map.size(), 1);
        
        char* value;
        int index = 0;
        for (auto const& x : map) {
            value = (char*) x.first.c_str();
            insertArrayStringItem(csound, strings, index, value);
            index ++;
        }
    }
};
struct jsonkeys : jsonkeysBase {
    PLUGINCHILD("S[]", "i", true)
};
struct jsonkeysK : jsonkeysBase {
    PLUGINCHILDK("S[]", "i", true)
};


/* 
 Get the size of a JSON object
 */
struct jsonsizeBase : plugin<1, 1> {    
    void run() {
        outargs[0] = (MYFLT) jsonSession->data.size();
    }
};
struct jsonsize : jsonsizeBase {
    PLUGINCHILD("i", "i", true)
};
struct jsonsizeK : jsonsizeBase {
    PLUGINCHILDK("k", "i", true)
};


/*
 Get object from an object by key name
 */
struct jsongetString : plugin<1, 2> {
    PLUGINIT("i", "iS", true)
    void irun() {
        STRINGDAT &input = inargs.str_data(1);
        jsoncons::json selected = jsonSession->data[std::string(input.data)];
        JSONSession* jsonSessionOutput;
        outargs[0] = createHandle<JSONSession>(csound, &jsonSessionOutput, handleName);
        jsonSessionOutput->active = true;
        jsonSessionOutput->data = selected;
    }
};


/*
 Get object from an array by index
 */
struct jsongetNumeric : plugin<1, 2> {
    PLUGINIT("i", "ii", true)
    void irun() {
        jsoncons::json selected = jsonSession->data[(int) inargs[1]];
        JSONSession* jsonSessionOutput;
        outargs[0] = createHandle<JSONSession>(csound, &jsonSessionOutput, handleName);
        jsonSessionOutput->active = true;
        jsonSessionOutput->data = selected;
    }
};


/*
 Query by JSONPath
 */
struct jsonpath : plugin<1, 2> {
	PLUGINIT("i", "iS", true)
    void irun() {
        jsoncons::json queried = jsoncons::jsonpath::json_query(
            jsonSession->data, std::string(inargs.str_data(1).data)
        );
        JSONSession* jsonSessionOutput;
        outargs[0] = createHandle<JSONSession>(csound, &jsonSessionOutput, handleName);
        jsonSessionOutput->active = true;
        jsonSessionOutput->data = queried;
    }
};


/*
 Replace string value by JSONPath
 */
struct jsonpathrplvalStringBase : inplug<3> {
	void run() {
        jsoncons::jsonpath::json_replace(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            std::string(args.str_data(2).data)
        );
	}
};
struct jsonpathrplvalString : jsonpathrplvalStringBase {
	INPLUGCHILD("iSS")
};
struct jsonpathrplvalStringK : jsonpathrplvalStringBase {
	INPLUGCHILDK("iSS")
};


/*
 Replace numeric value by JSONPath
 */
struct jsonpathrplvalNumericBase : inplug<3> {	
	void run() {
        jsoncons::jsonpath::json_replace(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            (float) args[2] // doesn't like double ??
        );
	}
};
struct jsonpathrplvalNumeric : jsonpathrplvalNumericBase {
	INPLUGCHILD("iSi")
};
struct jsonpathrplvalNumericK : jsonpathrplvalNumericBase {
	INPLUGCHILDK("iSi")
};


/*
 Replace element with object by JSONPath
 NOT SUPPORTED
struct jsonpathrpl : inplug<3> {
	INPLUGINIT("iSi")
	void irun() {
        JSONSession* jsonSession2;
        getSession(args[2], &jsonSession2);
        jsoncons::jsonpath::json_replace(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            jsonSession2->data
        );
	}
};
*/

/*
 Query by JSON Pointer
 */
struct jsonptr : plugin<1, 2> {
	PLUGINIT("i", "iS", true)
    void irun() {
        jsoncons::json queried = jsoncons::jsonpointer::get(
            jsonSession->data, std::string(inargs.str_data(1).data)
        );
        JSONSession* jsonSessionOutput;
        outargs[0] = createHandle<JSONSession>(csound, &jsonSessionOutput, handleName);
        jsonSessionOutput->active = true;
        jsonSessionOutput->data = queried;
    }
};

/*
 Check for existence by JSON Pointer
 */
struct jsonptrhasBase : plugin<1, 2> {
    void run() {
        outargs[0] = (int) jsoncons::jsonpointer::contains(
            jsonSession->data, std::string(inargs.str_data(1).data)
        );
    }
};
struct jsonptrhas : jsonptrhasBase {
	PLUGINCHILD("i", "iS", true)
};
struct jsonptrhasK : jsonptrhasBase {
	PLUGINCHILDK("k", "iS", true)
};


/*
 Add string value by JSON Pointer
 */
struct jsonptraddvalStringBase : inplug<3> {
	void run() {
        jsoncons::jsonpointer::add(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            std::string(args.str_data(2).data),
            true // create if not exists
        );
	}
};
struct jsonptraddvalString : jsonptraddvalStringBase {
    INPLUGCHILD("iSS")
};
struct jsonptraddvalStringK : jsonptraddvalStringBase {
    INPLUGCHILDK("iSS")
};


/*
 Add numeric value by JSON Pointer
 */
struct jsonptraddvalNumericBase : inplug<3> {
	void run() {
        jsoncons::jsonpointer::add(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            args[2],
            true // create if not exists
        );
	}
};
struct jsonptraddvalNumeric : jsonptraddvalNumericBase {
	INPLUGCHILD("iSi")
};
struct jsonptraddvalNumericK : jsonptraddvalNumericBase {
	INPLUGCHILDK("iSk")
};


/*
 Add object by JSON Pointer
 */
struct jsonptradd : inplug<3> {
	INPLUGINIT("iSi")
	void irun() {
        JSONSession* jsonSession2;
        getSession(args[1], &jsonSession2);
        
        jsoncons::jsonpointer::add(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            jsonSession2->data,
            true // create if not exists
        );
	}
};


/*
 Remove by JSON Pointer
 */
struct jsonptrrmBase : inplug<2> {
	void run() {
        char* query = args.str_data(1).data;
        jsoncons::jsonpointer::remove(jsonSession->data, std::string(query));
	}
};
struct jsonptrrm : jsonptrrmBase {
	INPLUGCHILD("iS")
};
struct jsonptrrmK : jsonptrrmBase {
	INPLUGCHILDK("iS")
};


/*
 Replace string value by JSON Pointer
 */
struct jsonptrrplvalStringBase : inplug<3> {
	void run() {
        jsoncons::jsonpointer::replace(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            std::string(args.str_data(2).data), 
            true // create if missing
        );
	}
};
struct jsonptrrplvalString : jsonptrrplvalStringBase {
	INPLUGCHILD("iSS")
};
struct jsonptrrplvalStringK : jsonptrrplvalStringBase {
	INPLUGCHILDK("iSS")
};


/*
 Replace numeric value by JSON Pointer
 */
struct jsonptrrplvalNumericBase : inplug<3> {
	void run() {
        jsoncons::jsonpointer::replace(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            args[2],
            true // create if missing
        );
	}
};
struct jsonptrrplvalNumeric : jsonptrrplvalNumericBase {
	INPLUGCHILD("iSi")
};
struct jsonptrrplvalNumericK : jsonptrrplvalNumericBase {
	INPLUGCHILDK("iSk")
};


/*
 Replace object by JSON Pointer
 */
struct jsonptrrpl : csnd::InPlug<3> {
    INPLUGSESSION
	INPLUGINIT("iSi")
	void irun() {
        JSONSession* jsonSession2;
        getSession(args[2], &jsonSession2);
        
        jsoncons::jsonpointer::replace(
            jsonSession->data, 
            std::string(args.str_data(1).data), 
            jsonSession2->data,
            true // create if missing
        );
	}
};


/*
 Get numeric array from object
 */
struct jsonarrvalNumericBase : plugin<1, 1> {
    void run() {
        jsonArrayToCSArray(csound, &(jsonSession->data), (ARRAYDAT*) outargs(0), false);         
    }
};
struct jsonarrvalNumeric : jsonarrvalNumericBase {
	PLUGINCHILD("i[]", "i", true)
};
struct jsonarrvalNumericK : jsonarrvalNumericBase {
	PLUGINCHILDK("k[]", "i", true)
};


/*
 Get string array from object
 */
struct jsonarrvalStringBase : plugin<1, 1> {
    void run() {
        jsonArrayToCSArray(csound, &(jsonSession->data), (ARRAYDAT*) outargs(0), true);         
    }
};
struct jsonarrvalString : jsonarrvalStringBase {
	PLUGINCHILD("S[]", "i", true)
};
struct jsonarrvalStringK : jsonarrvalStringBase {
	PLUGINCHILDK("S[]", "i", true)
};


/*
 Get array of object handles from object
 */
struct jsonarr : plugin<1, 1> {
    PLUGINIT("i[]", "i", true)
    void irun() {
        JSONSession* jsonSession2;
        std::vector<jsoncons::json> vals = 
                jsonSession->data.as<std::vector<jsoncons::json>>();
        ARRAYDAT* array = (ARRAYDAT*) outargs(0);
        arrayInit(csound, array, vals.size(), 1);
        MYFLT handle;
        for (std::size_t index = 0; index < vals.size(); index++) {
            handle = createHandle<JSONSession>(csound, &jsonSession2, handleName);
            jsonSession2->active = true;
            jsonSession2->data = vals[index];
            array->data[index] = handle;
        }
    }
};


/*
 Dump to string
 */
struct jsondumpsBase : plugin<1, 2> {
	void run() {
        STRINGDAT &output = outargs.str_data(0);
        std::ostringstream stream;
        if (inargs[1] == FL(1)) {
            stream << jsoncons::pretty_print(jsonSession->data);
        } else {
            stream << jsonSession->data;
        }
        char* text = csound->strdup((char*) stream.str().c_str());
        output.size = strlen(text);
        output.data = text;
	}
};
struct jsondumps : jsondumpsBase {
	PLUGINCHILD("S", "ip", true)
};
struct jsondumpsK : jsondumpsBase {
	PLUGINCHILDK("S", "ip", true)
};


/*
 Destroy object and clear memory
 */
struct jsondestroy : inplug<1> {
    INPLUGINIT("i")
    void irun() {
        jsonSession->data.~basic_json();
        jsonSession->active = false;
    }
};


/*
 Load from file
 */
struct jsonload : plugin<1, 1> {
	PLUGINIT("i", "S", false)
	void irun() {
        std::ifstream fileStream(inargs.str_data(0).data);
        jsoncons::json parsed = jsoncons::json::parse(fileStream);
        outargs[0] = createHandle<JSONSession>(csound, &jsonSession, handleName);
        jsonSession->active = true;
        jsonSession->data = parsed;
	}
};


/*
 Serialise to file
 */
struct jsondump : inplug<3> {
	INPLUGINIT("iSp")
	void irun() {
        std::ofstream fileStream;
        fileStream.open(args.str_data(1).data, std::ios::in | std::ios::trunc);
        if (!fileStream.is_open()) {
            throw std::runtime_error("could not open file for writing");
        }
        if (args[2] == FL(1)) {
            fileStream << jsoncons::pretty_print(jsonSession->data);
        } else {
            fileStream << jsonSession->data;
        }
        fileStream.close();
	}
};


#include <modload.h>
void csnd::on_load(csnd::Csound *csound) {
    csnd::plugin<jsoninit>(csound, "jsoninit", csnd::thread::i);
    csnd::plugin<jsonloads>(csound, "jsonloads", csnd::thread::i);
    csnd::plugin<jsondumps>(csound, "jsondumps", csnd::thread::i);
    csnd::plugin<jsondestroy>(csound, "jsondestroy", csnd::thread::i);
    csnd::plugin<jsondumpsK>(csound, "jsondumpsk", csnd::thread::ik);
    csnd::plugin<jsonload>(csound, "jsonload", csnd::thread::i);
    csnd::plugin<jsondump>(csound, "jsondump", csnd::thread::i);
    csnd::plugin<jsonmerge>(csound, "jsonmerge", csnd::thread::i);
    csnd::plugin<jsontype>(csound, "jsontype", csnd::thread::i);
    csnd::plugin<jsontypeString>(csound, "jsontype.S", csnd::thread::i);
    csnd::plugin<jsonkeys>(csound, "jsonkeys", csnd::thread::i);
    csnd::plugin<jsonkeysK>(csound, "jsonkeysk", csnd::thread::ik);
    csnd::plugin<jsongetString>(csound, "jsonget.S", csnd::thread::i);
    csnd::plugin<jsongetNumeric>(csound, "jsonget.i", csnd::thread::i);
    csnd::plugin<jsonsize>(csound, "jsonsize", csnd::thread::i);
    csnd::plugin<jsonsizeK>(csound, "jsonsizek", csnd::thread::ik);
  
    csnd::plugin<jsoninsert>(csound, "jsoninsert", csnd::thread::i);
    csnd::plugin<jsoninsertArray>(csound, "jsoninsert.a", csnd::thread::i);
    csnd::plugin<jsoninsertvalString>(csound, "jsoninsertval.S", csnd::thread::i);
    csnd::plugin<jsoninsertvalStringK>(csound, "jsoninsertvalk.S", csnd::thread::ik);
    csnd::plugin<jsoninsertvalNumeric>(csound, "jsoninsertval.i", csnd::thread::i);    
    csnd::plugin<jsoninsertvalNumericK>(csound, "jsoninsertvalk", csnd::thread::ik);    
    csnd::plugin<jsoninsertvalStringArray>(csound, "jsoninsertval.Sa", csnd::thread::i);
    csnd::plugin<jsoninsertvalStringArrayK>(csound, "jsoninsertvalk.Sa", csnd::thread::ik);
    csnd::plugin<jsoninsertvalNumericArray>(csound, "jsoninsertval.ia", csnd::thread::i);
    csnd::plugin<jsoninsertvalNumericArrayK>(csound, "jsoninsertvalk.a", csnd::thread::ik);
    csnd::plugin<jsoninsertvalStringStringArray>(csound, "jsoninsertval.SaSa", csnd::thread::i);
    csnd::plugin<jsoninsertvalStringStringArrayK>(csound, "jsoninsertvalk.SaSa", csnd::thread::ik);
    csnd::plugin<jsoninsertvalStringNumericArray>(csound, "jsoninsertval.Saia", csnd::thread::i);
    csnd::plugin<jsoninsertvalStringNumericArrayK>(csound, "jsoninsertvalk.Saka", csnd::thread::ik);
    
    csnd::plugin<jsonpath>(csound, "jsonpath", csnd::thread::i);
    csnd::plugin<jsonpathrplvalString>(csound, "jsonpathrplval.S", csnd::thread::i);
    csnd::plugin<jsonpathrplvalStringK>(csound, "jsonpathrplvalk.S", csnd::thread::ik);
    csnd::plugin<jsonpathrplvalNumeric>(csound, "jsonpathrplval.i", csnd::thread::i);
    csnd::plugin<jsonpathrplvalNumericK>(csound, "jsonpathrplvalk.i", csnd::thread::ik);
//    csnd::plugin<jsonpathrpl>(csound, "jsonpathrpl", csnd::thread::i);
    
    csnd::plugin<jsonptr>(csound, "jsonptr", csnd::thread::i);
    csnd::plugin<jsonptrhas>(csound, "jsonptrhas", csnd::thread::i);
    csnd::plugin<jsonptrhasK>(csound, "jsonptrhask", csnd::thread::ik);
    csnd::plugin<jsonptraddvalString>(csound, "jsonptraddval.S", csnd::thread::i);
    csnd::plugin<jsonptraddvalStringK>(csound, "jsonptraddvalk.S", csnd::thread::ik);
    csnd::plugin<jsonptraddvalNumeric>(csound, "jsonptraddval.i", csnd::thread::i);
    csnd::plugin<jsonptraddvalNumericK>(csound, "jsonptraddvalk.i", csnd::thread::ik);
    csnd::plugin<jsonptradd>(csound, "jsonptradd", csnd::thread::i);
    csnd::plugin<jsonptrrm>(csound, "jsonptrrm", csnd::thread::i);
    csnd::plugin<jsonptrrmK>(csound, "jsonptrrmk", csnd::thread::ik);
    csnd::plugin<jsonptrrplvalString>(csound, "jsonptrrplval.S", csnd::thread::i);
    csnd::plugin<jsonptrrplvalStringK>(csound, "jsonptrrplvalk.S", csnd::thread::ik);
    csnd::plugin<jsonptrrplvalNumeric>(csound, "jsonptrrplval.i", csnd::thread::i);
    csnd::plugin<jsonptrrplvalNumericK>(csound, "jsonptrrplvalk.i", csnd::thread::ik);
    csnd::plugin<jsonptrrpl>(csound, "jsonptrrpl", csnd::thread::i);
    
    csnd::plugin<jsonarrvalString>(csound, "jsonarrval.S", csnd::thread::i);
    csnd::plugin<jsonarrvalStringK>(csound, "jsonarrvalk.S", csnd::thread::ik);
    csnd::plugin<jsonarrvalNumeric>(csound, "jsonarrval.i", csnd::thread::i);
    csnd::plugin<jsonarrvalNumericK>(csound, "jsonarrval.k", csnd::thread::ik);
    csnd::plugin<jsonarr>(csound, "jsonarr", csnd::thread::i);
}
