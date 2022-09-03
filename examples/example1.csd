/*
    csound-json example 1

    load data, examine and modify, print and output to file

*/
<CsoundSynthesizer>
<CsLicence>
    Released into the public domain under the Unlicense license
    http://unlicense.org/
</CsLicence>
<CsOptions>
-odac
-d
</CsOptions>
<CsInstruments>
sr = 44100
ksmps = 64
nchnls = 2
0dbfs = 1


; base JSON string
gSjson = {{
    {
        "instruments": {
            "oscil1": [
                [0, 2, 440],
                [2, 4, 880],
                [4, 2, 220]
            ],
            "oscil2": [
                [6, 2, 880, "word"],
                [8, 2, 220]
            ]
        },
        "details": {
            "createdby": "Richard Knight",
            "createdon": "2022-09-02"
        }
    }
}}

instr boot

    ; load JSON data and print the type detected
	iJson jsonloads gSjson
    prints sprintf("1. Loaded; type = %s\n", jsontype:S(iJson))

    ; query using JSONPath, print type found and dump string
    iqueried jsonpath iJson, "$.instruments['oscil2'][0][:]"
    prints sprintf("2. Path query 1; type = %s\n", jsontype:S(iqueried))
    prints sprintf("3. Dumped path query 1:\n%s\n\n", jsondumps(iqueried))

    ; output JSONPath query result to numeric array (strings result in 0)
    prints "4. Path query 1 to numeric array:\n"
    idata[] jsonarrval iqueried
    printarray idata

    ; output JSONPath query result to string array
    prints "\n5. Path query 1 to string array:\n"
    Sarray[] jsonarrval iqueried
    index = 0
    while (index < lenarray(Sarray)) do
        prints sprintf("%s ", Sarray[index])
        index += 1
    od
    prints "\n\n"

    ; query with JSON Pointer, print type and dump string
    iqueried jsonptr iJson, "/instruments/oscil1"
    prints sprintf("6. Pointer query 1; type = %s\n", jsontype:S(iqueried))
    prints sprintf("7. output:\n%s\n\n", jsondumps(iqueried))

    ; query with JSON Pointer, print type (should be numeric), convert and print
    iqueried jsonptr iJson, "/instruments/oscil1/0/2"
    prints sprintf("8. Pointer query 2; type = %s\n", jsontype:S(iqueried))
    inum = strtod(jsondumps(iqueried))
    prints sprintf("9. output as string, converted to numeric in csound: %d\n\n", inum)

    ; query with JSON Pointer to get instruments object
    iinstruments1 jsonptr iJson, "/instruments"
    
    ; load supplementary data from file and extract instruments object with JSON Pointer query
    iJson2 jsonload "supplement.json"
    prints sprintf("10. File loaded; type = %s\n", jsontype:S(iJson2))
    iinstruments2 jsonptr iJson2, "/instruments"
    

    ; merge the two instruments objects
    jsonmerge iinstruments1, iinstruments2, 1
    prints sprintf("11. Merged instruments; dumped data:\n%s\n\n", jsondumps(iinstruments1))

    ; insert the merged object back to the "instruments" key
    jsoninsert iJson, "instruments", iinstruments1
    prints sprintf("12. Inserted instruments back to original; dumped data:\n%s\n\n", jsondumps(iJson))

    ; create a new empty JSON object and insert some values with keys
    iJson jsoninit
    jsoninsertval iJson, "hello", "world"
    jsoninsertval iJson, "numeric", 3

    ; create JSON object from string and insert with key
    jsoninsert iJson, "array", jsonloads("[1,2,3]")

    ; insert array values
    iarraydata[] fillarray 3, 2, 1, 0
    jsoninsertval iJson, "arraycs", iarraydata

    ; insert string array values
    Sarraydata[] fillarray "hello", "world", "again"
    jsoninsertval iJson, "stringarraycs", Sarraydata

    iJsonarray[] fillarray jsonloads("{\"a\": \"b\"}"), jsonloads("{\"c\": 321, \"e\": 123}")
    jsoninsert iJson, "subobject", iJsonarray

    ; use JSON Pointer to replace a value
    jsonptrrplval iJson, "/hello", "goodbye"

    ; use JSONPath to replace value
    jsonpathrplval iJson, "$.subobject[0]['a']", "this is a"

    prints sprintf("13. Inserted to new; dumped data:\n%s\n\n", jsondumps(iJson))

    jsondump iJson, "example_output.json"
    prints "14. Wrote to output file\n"
endin

</CsInstruments>
<CsScore>
i"boot" 0 1
</CsScore>
</CsoundSynthesizer>
