/*
    csound-json example 2

    merge objects, play score based on contents

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
                [6, 2, 330],
                [8, 2, 660]
            ]
        },
        "details": {
            "createdby": "Richard Knight",
            "createdon": "2022-09-02"
        }
    }
}}


instr boot

    ; load the string and the supplementary file
	iJbase jsonloads gSjson
    iJsupplement jsonload "supplement.json"

    ; get the instruments objects and merge them
    iJinstrs1 jsonget iJbase, "instruments"
    iJinstrs2 jsonget iJsupplement, "instruments"
    jsonmerge iJinstrs1, iJinstrs2, 1
    
    ; get keys of the resulting instruments
    Skeys[] jsonkeys iJinstrs1

    ; loop through the keys
    indexi = 0
    while (indexi < lenarray(Skeys)) do
        Sinstrument = Skeys[indexi]

        ; get the relevant instrument object
        iJscore jsonget iJinstrs1, Sinstrument

        ; score items are retrieved as handles to JSON arrays, in a Csound array
        iJscorelines[] jsonarr iJscore

        ; loop through each of the JSON arrays
        indexs = 0
        while (indexs < lenarray(iJscorelines)) do

            ; get the JSON array values and format/call the scoreline accordingly
            isc[] jsonarrval iJscorelines[indexs]
            Scoreline = sprintf("i\"%s\" %d %d %d\n", Sinstrument, isc[0], isc[1], isc[2])
            prints Scoreline
            scoreline_i Scoreline
            indexs += 1
        od
        indexi += 1
    od
endin


/*
    The sound producting instruments to be called
*/
instr oscil1
	ifreq = p4
	kamp linseg 1, p3*0.7, 1, p3*0.3, 0
	aout oscil kamp, ifreq, 1
	outs aout, aout
endin

instr oscil2
	ifreq = p4
	kamp linseg 1, p3*0.7, 1, p3*0.3, 0
	aout oscil kamp, ifreq, 2
	outs aout, aout
endin

instr oscil3
	ifreq = p4
	kamp linseg 1, p3*0.7, 1, p3*0.3, 0
	aout oscil kamp, ifreq, 3
	outs aout, aout
endin


</CsInstruments>
<CsScore>
f1 0 16384 10 1                                          ; Sine
f2 0 16384 10 1 0.5 0.3 0.25 0.2 0.167 0.14 0.125 .111   ; Sawtooth
f3 0 16384 10 1 0   0.3 0    0.2 0     0.14 0     .111   ; Square
f0 15
i"boot" 0 1
</CsScore>
</CsoundSynthesizer>
