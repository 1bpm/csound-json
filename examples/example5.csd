/*
    csound-json example 5

    fill JSON object with various data at k-rate and init time
        add to an object loaded from disk
        add f-table to JSON
        print and save to disk

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


/*
    Create a random alphabetic string
*/
opcode randstring, S, 0
    Soutput = sprintf("%c%c%c%c%c%c", \
        random(97, 122), random(97, 122),\
        random(97, 122), random(97, 122),\
        random(97, 122), random(97, 122)\
    )
    xout Soutput
endop


instr boot

    ; empty objects
    iJson1 jsoninit
    iJson2 jsoninit

    ; fill with data
    schedule "run1", 0, 5, iJson1
    schedule "run2", 5, 5, iJson2

    ; combine objects
    schedule "process", 10, 1, iJson1, iJson2
    turnoff
endin


instr run1
    iJson = p4
    kdata[] init 4
    kindex init 1
    kmetro metro 5

    ; add random numeric values with incremental key names
    if (kmetro == 1) then
        Skey = sprintfk("sound%03d", kindex)
        kdata[0] = random:k(0, 1)
        kdata[1] = random:k(0, 1)
        kdata[2] = random:k(0, 1)
        kdata[3] = random:k(0, 1)
        jsoninsertvalk iJson, Skey, kdata
        kindex += 1
    endif
endin


instr run2
    iJson = p4
    Skeys[] init 3
    Svalues[] init 3

    ; add random key/value strings
    index = 0
    while (index < 10) do
        Skeys[0] randstring
        Skeys[1] randstring
        Skeys[2] randstring
        Svalues[0] randstring
        Svalues[1] randstring
        Svalues[2] randstring
        jsoninsertval iJson, Skeys, Svalues
        index += 1
    od
endin


instr process

    ; populated objects
    iJsons[] fillarray p4, p5

    ; load file
    iJson jsonload "supplement.json"

    ; insert as array of objects under "added" key
    jsoninsert iJson, "added", iJsons

    ; add f-table sine wave points
    ipoints[] tab2array 1
    jsoninsertval iJson, "sine", ipoints

    ; show and write output
    prints sprintf("%s\n\n", jsondumps(iJson))
    jsondump iJson, "example_output.json"
endin


   
</CsInstruments>
<CsScore>
f1 0 64 10 1    ; sine
i"boot" 0 15
</CsScore>
</CsoundSynthesizer>