

# csound-json : JSON parsing and manipulation for Csound

## Overview
csound-json provides over fifty permutations of opcodes for the parsing and manipulation of JSON. Data can be deserialised from a string or file, and similarly serialised and output to string or file. 
Simple key/index access is available, as are more powerful query expressions using JSONPath and JSON Pointer. Arrays can be easily translated to native Csound arrays and reinserted to JSON structures. Mostly all operations are performed on object handles, which are Csound references to JSON objects stored internally. All opcodes are available at init time, and those feasible for k-rate performance are additionally provided.

## Requirements
* Cmake >= 2.8.12
* Csound with development headers >= 6.14.0
* Compiler with C++11 support 
	* If using G++, >= 4.8.5 is needed due to std::regex implementation



## Installation
Create a build directory at the top of the source tree, execute *cmake ..*, *make* and optionally *make install* as root. If the latter is not used/possible then the resulting library can be used with the *--opcode-lib* flag in Csound.
eg:

    git clone https://git.1bpm.net/csound-json
	cd csound-json
	mkdir build && cd build
	cmake ..
	make && sudo make install

Cmake should find Csound and any other required libraries using the modules in the cmake/Modules directory and installation should be as simple as above.

## Examples
Some examples are provided in the examples directory.


## Opcode reference


### jsonloads
Parse a JSON string and load to an object handle for use in other opcodes.

	iJson jsonloads Sjson
* **iJson** loaded JSON object handle
* **Sjson** string to parse


### jsonload
Parse JSON from a file and load to an object handle for use in other opcodes.

	iJson jsonload Sfile
* **iJson** loaded JSON object handle
* **Sfile** file path containing JSON data


### jsondumps
Output a JSON object handle as a string.

	Soutput jsondumps iJson [, ipretty=1]
* **Soutput** the serialised object contents
* **iJson** JSON object handle to evaluate
* **ipretty** 1=pretty print with formatting and indenting, 0=raw


### jsondumpsk
Output a JSON object handle as a string at k-rate

	Soutput jsondumpsk iJson [, ipretty=1]
* **Soutput** the serialised object contents, at k-rate
* **iJson** JSON object handle to evaluate
* **ipretty** 1=pretty print with formatting and indenting, 0=raw


### jsondump
Output a JSON object handle to a text file.

	jsondump iJson, Sfile [, ipretty=1]
* **iJson** JSON object handle to evaluate
* **Sfile** file path to write serialised object contents to
* **ipretty** 1=pretty print with formatting and indenting, 0=raw


### jsoninit
Initialise an empty JSON object (equivalent to `iJson jsonloads "{}"`).

	iJson jsoninit
* **iJson** new empty object


### jsondestroy
Delete a JSON object and free memory.

	jsondestroy iJson
* **iJson** JSON object handle to destroy


### jsonmerge
Shallow merge two JSON object handles, from *iJsonSource* into *iJsonTarget*. If *iupdate* = 1, then any existing keys will be altered, otherwise existing keys will not be merged.

	jsonmerge iJsonTarget, iJsonSource [, iupdate=0]	
* **iJsonTarget** JSON object handle to be merged into.
* **iJsonSource** JSON object handle to be merged from.
* **iupdate** 1=update and replace any keys that exist in *iJsonTarget*; 0=skip merging existing keys.


### jsoninsert
Insert a JSON object handle to another JSON object handle with a specified key. An array of JSON object handles can be provided as *iJsonInsert[]*, which are then inserted as their relevant types under the key *Skey*.

	jsoninsert iJson, Skey, iJsonInsert
	jsoninsert iJson, Skey, iJsonInsert[]
* **iJson** JSON object handle to insert to
* **Skey** key name under which the objects will be added
* **iJsonInsert** single JSON object handle
* **iJsonInsert[]** array of JSON object handles


### jsoninsertval
Insert a single value or a set of values to a JSON object handle.
When *Skeys[]* and (*Svalues[]* or *ivalues[]*) are both provided, the array lengths must match and are inserted as a set of individual key/value pairs.
When *Skey* and (*Svalues[]* or *ivalues[]*) are provided, the values are inserted into *Skey* as an array.

	jsoninsertval iJson, Skey, Svalue
	jsoninsertval iJson, Skey, ivalue
	jsoninsertval iJson, Skey, ivalues[]
	jsoninsertval iJson, Skey, Svalues[]
	jsoninsertval iJson, Skeys[], ivalues[]
	jsoninsertval iJson, Skeys[], Svalues[]
* **iJson** JSON object handle to insert to
* **Skey** key name under which the value(s) will be added
* **Svalue** string value to be added
* **ivalue** numeric value to be added
* **Svalues[]** array of string values to be added
* **ivalues[]** array of numeric values to be added
* **Skeys[]** array of keys to be used for key/value operation


### jsoninsertvalk
Insert a single value or a set of values to a JSON object handle, at k-rate
When *Skeys[]* and (*Svalues[]* or *kvalues[]*) are both provided, the array lengths must match and are inserted as a set of individual key/value pairs.
When *Skey* and (*Svalues[]* or *kvalues[]*) are provided, the values are inserted into *Skey* as an array.

	jsoninsertvalk iJson, Skey, Svalue
	jsoninsertvalk iJson, Skey, kvalue
	jsoninsertvalk iJson, Skey, kvalues[]
	jsoninsertvalk iJson, Skey, Svalues[]
	jsoninsertvalk iJson, Skeys[], kvalues[]
	jsoninsertvalk iJson, Skeys[], Svalues[]
* **iJson** JSON object handle to insert to
* **Skey** key name under which the value(s) will be added
* **Svalue** string value to be added
* **kvalue** numeric value to be added
* **Svalues[]** array of string values to be added
* **kvalues[]** array of numeric values to be added
* **Skeys[]** array of keys to be used for key/value operation


### jsontype
Get the type of a JSON object handle.

	itype jsontype iJson
	Stype jsontype iJson
* **itype** numeric type:
	* -1 unknown
	* 0 null
	* 1 string
	* 2 number
	* 3 boolean
	* 4 array
	* 5 object
* **Stype** string type, as listed above
* **iJson** JSON object handle to evaluate


### jsonkeys
Get the keys of a JSON object handle, if available (ie, if the type is *object*).

	Skeys[] jsonkeys iJson
* **Skeys[]** the keys
* **iJson** JSON object handle to evaluate


### jsonkeysk
Get the keys of a JSON object handle at k-rate, if available (ie, if the type is *object*).

	Skeys[] jsonkeysk iJson
* **Skeys[]** the keys, at k-rate
* **iJson** JSON object handle to evaluate


### jsonsize
Get the size of a JSON object handle (ie, the array size or number of object keys).
	
	isize jsonsize iJson
* **isize** number of elements
* ** iJson** JSON object handle to evaluate


### jsonsizek
Get the size of a JSON object handle at k-rate (ie, the array size or number of object keys).
	
	ksize jsonsizek iJson
* **ksize** number of elements
* ** iJson** JSON object handle to evaluate


### jsonget
Get a JSON object handle of the object contained in the specified key or index.

	iJsonOutput jsonget iJson, Skey
	iJsonOutput jsonget iJson, index
* **iJsonOutput** the JSON object handle as contained in *Skey* or *index*
* **iJson** JSON object handle to evaluate
* **Skey** key for accessing an object
* **index** index for accessing an array


### jsonpath
Perform a JSONPath query and obtain the resulting JSON object handle.

	iJsonOutput jsonpath iJson, Spath
* **iJsonOutput** JSON object handle specified by *Spath*
* **iJson** JSON object handle to evaluate
* **Spath** JSONPath expression


### jsonpathrplval
Replace a value in a location specified by the JSONPath expression *Spath*

	jsonpathrplval iJson, Spath, ivalue
	jsonpathrplval iJson, Spath, Svalue
* **iJson** JSON object handle to evaluate
* **Spath** JSONPath expression
* **ivalue** numeric value to replace target with
* **Svalue** string value to replace target with


### jsonpathrplvalk
Replace a value in a location specified by the JSONPath expression *Spath* at k-rate.

	jsonpathrplvalk iJson, Spath, kvalue
	jsonpathrplvalk iJson, Spath, Svalue
* **iJson** JSON object handle to evaluate
* **Spath** JSONPath expression
* **kvalue** numeric value to replace target with
* **Svalue** string value to replace target with


### jsonpathrpl
Replace a JSON object specified by the JSONPath expression *Spath*

	jsonpathrpl iJson, Spath, iJsonNew
* **iJson** JSON object handle to evaluate
* **Spath** JSONPath expression
* **iJsonNew** JSON object handle to replace target with


### jsonptr
Perform a JSON Pointer query and obtain the resulting JSON object handle.

	iJsonOutput jsonptr iJson, Spointer
* **iJsonOutput** JSON object handle specified by *Spointer*
* **iJson** JSON object handle to evaluate
* **Spointer** JSON Pointer expression


### jsonptrhas
Check if a JSON Pointer query results in a valid existing object.

	iexists jsonptrhas iJson, Spointer
* **iexists** 1 if existing, 0 if not
* **iJson** JSON object handle to evaluate
* **Spointer** JSON Pointer expression


### jsonptrhask
Check if a JSON Pointer query results in a valid existing object, at k-rate

	kexists jsonptrhask iJson, Spointer
* **kexists** 1 if existing, 0 if not
* **iJson** JSON object handle to evaluate
* **Spointer** JSON Pointer expression


### jsonptraddval
Add a value at a location specified by the JSON Pointer expression *Spointer*.

	jsonptraddval iJson, Spointer, ivalue
	jsonptraddval iJson, Spointer, Svalue
* **iJson** JSON object handle to add value to
* **Spointer** JSON Pointer expression
* **ivalue** numeric value to add
* **Svalue** string value to add


### jsonptraddvalk
Add a value at a location specified by the JSON Pointer expression *Spointer*.

	jsonptraddvalk iJson, Spointer, kvalue
	jsonptraddvalk iJson, Spointer, Svalue
* **iJson** JSON object handle to add value to
* **Spointer** JSON Pointer expression
* **kvalue** numeric value to add
* **Svalue** string value to add


### jsonptradd
Add a JSON object handle to a location specified by the JSON Pointer expression *Spointer*.

	jsonptradd iJson, Spointer, iJsonNew

* **iJson** JSON object handle to add to
* **Spointer** JSON pointer expression
* **iJsonNew** JSON object handle to add to *iJson*


### jsonptrrm
Remove an object specified by the JSON Pointer expression *Spointer*.

	jsonptrrm iJson, Spointer
* **iJson** JSON object handle to remove from
* **Spointer** JSON pointer expression


### jsonptrrmk
Remove an object specified by the JSON Pointer expression *Spointer*, at k-rate.

	jsonptrrmk iJson, Spointer
* **iJson** JSON object handle to remove from
* **Spointer** JSON pointer expression, at k-rate


### jsonptrrplval
Replace a value specified by the JSON Pointer expression *Spointer*.

	jsonptrrplval iJson, Spointer, ivalue
	jsonptrrplval iJson, Spointer, Svalue
* **iJson** JSON object handle to replace in
* **Spointer** JSON Pointer expression
* **ivalue** numeric value to set
* **Svalue** string value to set


### jsonptrrplvalk
Replace a value specified by the JSON Pointer expression *Spointer*, at k-rate.

	jsonptrrplvalk iJson, Spointer, kvalue
	jsonptrrplvalk iJson, Spointer, Svalue
* **iJson** JSON object handle to replace in
* **Spointer** JSON Pointer expression
* **kvalue** numeric value to set
* **Svalue** string value to set


### jsonptrrpl
Replace an object specified by the JSON Pointer expression *Spointer*.

	jsonptrrpl iJson, Spointer, iJsonNew
* **iJson** JSON object handle to replace in
* **Spointer** JSON Pointer expression
* **iJsonNew** JSON object handle to set


### jsonarrval
Get an array of values from a JSON object handle.

	ivalues[] jsonarrval iJson
	Svalues[] jsonarrval iJson
* **ivalues[]** returned numeric values
* **Svalues[]** returned string values
* **iJson** JSON object handle to evaluate


### jsonarrvalk
Get an array of values from a JSON object handle, at k-rate.

	kvalues[] jsonarrvalk iJson
	Svalues[] jsonarrvalk iJson
* **kvalues[]** returned numeric values
* **Svalues[]** returned string values
* **iJson** JSON object handle to evaluate


### jsonarr
Get an array of JSON object handles from a JSON object handle.

	iJsonObjects[] jsonarr iJson
* **iJsonObjects[]** array of JSON object handles
* **iJson** JSON object handle to evaluate



## Links
* JSON Pointer is standardised in a [RFC specification](https://www.rfc-editor.org/rfc/rfc6901).
* JSONPath is not standardised but has an original [specification](https://goessner.net/articles/JsonPath/) and an [IETF standardisation](https://datatracker.ietf.org/wg/jsonpath/about/) working group in progress as of writing.
* JSONPath and JSON Pointer online evaluators may be helpful for authoring queries, [here is one such tool](https://www.jsonquerytool.com/).
* csound-json is powered by [jsoncons](https://github.com/danielaparker/jsoncons).

