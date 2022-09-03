/*
    csound-json example 4

    fill JSON object with various data at init time

*/
<CsoundSynthesizer>
<CsLicence>
    Released into the public domain under the Unlicense license
    http://unlicense.org/
</CsLicence>
<CsOptions>
-d
-m0
</CsOptions>
<CsInstruments>
sr = 44100
ksmps = 64
nchnls = 2
0dbfs = 1


instr boot

    ; empty object
    iJson jsoninit

    ; add key/value combination as "description" object
    Skeys[] fillarray "colour", "taste", "smell"
    Svalues[] fillarray "blue", "sweet", "vile"
    iJsonSub1 jsoninit
    jsoninsertval iJsonSub1, Skeys, Svalues
    jsoninsert iJson, "description", iJsonSub1

    ; add key/value combination and additional value to iJsonSub2
    Skeys[] fillarray "height", "width"
    ivalues[] fillarray 35.4, 6.41
    iJsonSub2 jsoninit
    jsoninsertval iJsonSub2, Skeys, ivalues
    jsoninsertval iJsonSub2, "depth", 16.439

    ; add iJsonSub2 to array along with new specified string objects
    iJsonObjects[] fillarray iJsonSub2, jsonloads("{\"not\": \"much\"}"), jsonloads("[1,2,3]")

    ; add all of the above iJsonObjects back into the main object under "measurements" key
    jsoninsert iJson, "measurements", iJsonObjects

    ; show the result
    prints sprintf("%s\n\n", jsondumps(iJson))
endin


</CsInstruments>
<CsScore>
i"boot" 0 1
</CsScore>
</CsoundSynthesizer>