/*
    csound-json example 3

    add values to an object at k-rate and print them after

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


instr write_values

    ; create empty object
    iJson jsoninit

    ; insert some values to the object four times a second
    kmetro metro 4
    kindex init 1
    if (kmetro == 1) then
        jsoninsertvalk iJson, sprintfk("key%03d", kindex), kindex*random:k(1, 10)
        kindex += 1
    endif

    ; print what has been inserted at the end
    schedule "print_values", p3, 1, iJson
endin


instr print_values
    iJson = p4

    ; dump JSON and print
    prints sprintf("%s\n\n", jsondumps(iJson))
endin


</CsInstruments>
<CsScore>
f0 11
i"write_values" 0 10
</CsScore>
</CsoundSynthesizer>