//////////////////////////////////////////////////////////////////////////
//
//     HALL A C++/ROOT Parity Analyzer  Pan           
//
//           DevTypes.hh  (definition file)
//           ^^^^^^^^^^^
//
//    Authors :  R. Holmes, A. Vacheret, R. Michaels
//
//  Defines arbitrary integer "keys" used to access data (both raw 
//  and calibrated) from fData array in TaEvent class.   
//  The keys have easy-to-remember names.  
//  Restrictions: 
//    1. the keys must be unique.
//    2. MAXKEYS must be > largest key.
//    3. ordering of keys affects TaEvent::Decode().
//
//////////////////////////////////////////////////////////////////////////

#define MAXKEYS  715
#define ADCREADOUT     1
#define SCALREADOUT    2

// Keys for getting data from devices.
// They keys are mapped to devices automatically.
// Use these keys for  TaEvent::GetData(Int_t key)

#define   STROFF        1         // Stripline BPMs start here
#define   STRNUM        7         // number of striplines defined below

#define   IBPM8XP       1         // BPM8 XP antenna
#define   IBPM8XM       2         // BPM8 XM antenna
#define   IBPM8YP       3         // BPM8 YP antenna
#define   IBPM8YM       4         // BPM8 YM antenna
#define   IBPM8X        5         // calibrated X position
#define   IBPM8Y        6         // calibrated Y position
#define   IBPM8XWS      7         // X wiresum
#define   IBPM8YWS      8         // Y wiresum
#define   IBPM8WS       9         // wiresum

#define   IBPM10XP     10          
#define   IBPM10XM     11
#define   IBPM10YP     12
#define   IBPM10YM     13
#define   IBPM10X      14          
#define   IBPM10Y      15          
#define   IBPM10XWS    16         
#define   IBPM10YWS    17         
#define   IBPM10WS     18         

#define   IBPM12XP     19          
#define   IBPM12XM     20
#define   IBPM12YP     21
#define   IBPM12YM     22
#define   IBPM12X      23          
#define   IBPM12Y      24          
#define   IBPM12XWS    25         
#define   IBPM12YWS    26         
#define   IBPM12WS     27         

#define   IBPM4AXP     28          
#define   IBPM4AXM     29
#define   IBPM4AYP     30
#define   IBPM4AYM     31
#define   IBPM4AX      32          
#define   IBPM4AY      33          
#define   IBPM4AXWS    34         
#define   IBPM4AYWS    35         
#define   IBPM4AWS     36         

#define   IBPM4BXP     37          
#define   IBPM4BXM     38
#define   IBPM4BYP     39
#define   IBPM4BYM     40
#define   IBPM4BX      41          
#define   IBPM4BY      42          
#define   IBPM4BXWS    43         
#define   IBPM4BYWS    44         
#define   IBPM4BWS     45         

// Injector striplines
#define   IBPMIN1XP    46          
#define   IBPMIN1XM    47
#define   IBPMIN1YP    48
#define   IBPMIN1YM    49
#define   IBPMIN1X     50          
#define   IBPMIN1Y     51          
#define   IBPMIN1XWS   52         
#define   IBPMIN1YWS   53         
#define   IBPMIN18WS   54         

#define   IBPMIN2XP    55
#define   IBPMIN2XM    56
#define   IBPMIN2YP    57
#define   IBPMIN2YM    58
#define   IBPMIN2X     59          
#define   IBPMIN2Y     60          
#define   IBPMIN2XWS   61         
#define   IBPMIN2YWS   62         
#define   IBPMIN2WS    63         

// Cavity BPMs
#define   CAVOFF       65          // Cavities start here
#define   CAVNUM        4          // number of cavities

#define   IBPMCAV1XR   65          // Cavity 1  X  raw data
#define   IBPMCAV1YR   66
#define   IBPMCAV1X    67          // Cavity 1  X calibrated position
#define   IBPMCAV1Y    68          

#define   IBPMCAV2XR   69 
#define   IBPMCAV2YR   70
#define   IBPMCAV2X    71 
#define   IBPMCAV2Y    72          

#define   IBPMCAV3XR   73 
#define   IBPMCAV3YR   74
#define   IBPMCAV3X    75 
#define   IBPMCAV3Y    76          

#define   IBPMCAV4XR   77 
#define   IBPMCAV4YR   78
#define   IBPMCAV4X    79 
#define   IBPMCAV4Y    80          

// Old cavities (Happex 1 era) for current
#define   BCMOFF       83
#define   BCMNUM        3

#define   IBCM1R       83          // raw bcm1  (note the "R" for raw)
#define   IBCM1        84          // calibrated bcm1

#define   IBCM2R       85          
#define   IBCM2        86          

#define   IBCM3R       87          
#define   IBCM3        88          

// G0 cavities BCMs
#define   CCMOFF       89
#define   CCMNUM        4

#define   IBCMCAV1R    89         // new G0 cavity BCM 1 raw data   (note the "R" for raw)
#define   IBCMCAV1     90         // new G0 cavity BCM 1 calibrated data

#define   IBCMCAV2R    91
#define   IBCMCAV2     92  

#define   IBCMCAV3R    93
#define   IBCMCAV3     94  

#define   IBCMCAV4R    95
#define   IBCMCAV4     96  
        
// Batteries 
#define   BATOFF      101
#define   BATNUM        5

#define   IBATT1      101         //  battery 1
#define   IBATT2      102         
#define   IBATT3      103         
#define   IBATT4      104         
#define   IBATT5      105

// Detectors 
#define   DETOFF      108
#define   DETNUM        4

#define   IDET1R      108         // detector 1  raw signal
#define   IDET1       109         // detector 1  calibrated signal

#define   IDET2R      110
#define   IDET2       111 

#define   IDET3R      112
#define   IDET3       113 

#define   IDET4R      114
#define   IDET4       115 

// ADC data.  Data are arranged in sequence starting with ADC0_0
// First index is the adc#, second is channel#.  Indices start at 0
// ADC# 0 - 9 are in first crate, 10 in next crate, etc.  

// First the raw data
#define   ADCOFF      120
#define   ADCNUM       15

#define   IADC0_0     120
#define   IADC0_1     121
#define   IADC0_2     122
#define   IADC0_3     123
#define   IADC1_0     124
#define   IADC1_1     125
#define   IADC1_2     126
#define   IADC1_3     127
#define   IADC2_0     128
#define   IADC2_1     129
#define   IADC2_2     130
#define   IADC2_3     131
#define   IADC3_0     132
#define   IADC3_1     133
#define   IADC3_2     134
#define   IADC3_3     135
#define   IADC4_0     136
#define   IADC4_1     137
#define   IADC4_2     138
#define   IADC4_3     139
#define   IADC5_0     140
#define   IADC5_1     141
#define   IADC5_2     142
#define   IADC5_3     143
#define   IADC6_0     144
#define   IADC6_1     145
#define   IADC6_2     146
#define   IADC6_3     147
#define   IADC7_0     148
#define   IADC7_1     149
#define   IADC7_2     150
#define   IADC7_3     151
#define   IADC8_0     152
#define   IADC8_1     153
#define   IADC8_2     154
#define   IADC8_3     155
#define   IADC9_0     156
#define   IADC9_1     157
#define   IADC9_2     158
#define   IADC9_3     159
#define   IADC10_0    160
#define   IADC10_1    161
#define   IADC10_2    162
#define   IADC10_3    163
#define   IADC11_0    164
#define   IADC11_1    165
#define   IADC11_2    166
#define   IADC11_3    167
#define   IADC12_0    168
#define   IADC12_1    169
#define   IADC12_2    170
#define   IADC12_3    171
#define   IADC13_0    172
#define   IADC13_1    173
#define   IADC13_2    174
#define   IADC13_3    175
#define   IADC14_0    176
#define   IADC14_1    177
#define   IADC14_2    178
#define   IADC14_3    179

// Now the calibrated data
#define   ACCOFF      200

#define   IADC_CAL0_0     200
#define   IADC_CAL0_1     201
#define   IADC_CAL0_2     202
#define   IADC_CAL0_3     203
#define   IADC_CAL1_0     204
#define   IADC_CAL1_1     205
#define   IADC_CAL1_2     206
#define   IADC_CAL1_3     207
#define   IADC_CAL2_0     208
#define   IADC_CAL2_1     209
#define   IADC_CAL2_2     210
#define   IADC_CAL2_3     211
#define   IADC_CAL3_0     212
#define   IADC_CAL3_1     213
#define   IADC_CAL3_2     214
#define   IADC_CAL3_3     215
#define   IADC_CAL4_0     216
#define   IADC_CAL4_1     217
#define   IADC_CAL4_2     218
#define   IADC_CAL4_3     219
#define   IADC_CAL5_0     220
#define   IADC_CAL5_1     221
#define   IADC_CAL5_2     222
#define   IADC_CAL5_3     223
#define   IADC_CAL6_0     224
#define   IADC_CAL6_1     225
#define   IADC_CAL6_2     226
#define   IADC_CAL6_3     227
#define   IADC_CAL7_0     228
#define   IADC_CAL7_1     229
#define   IADC_CAL7_2     230
#define   IADC_CAL7_3     231
#define   IADC_CAL8_0     232
#define   IADC_CAL8_1     233
#define   IADC_CAL8_2     234
#define   IADC_CAL8_3     235
#define   IADC_CAL9_0     236
#define   IADC_CAL9_1     237
#define   IADC_CAL9_2     238
#define   IADC_CAL9_3     239
#define   IADC_CAL10_0    240
#define   IADC_CAL10_1    241
#define   IADC_CAL10_2    242
#define   IADC_CAL10_3    243
#define   IADC_CAL11_0    244
#define   IADC_CAL11_1    245
#define   IADC_CAL11_2    246
#define   IADC_CAL11_3    247
#define   IADC_CAL12_0    248
#define   IADC_CAL12_1    249
#define   IADC_CAL12_2    250
#define   IADC_CAL12_3    251
#define   IADC_CAL13_0    252
#define   IADC_CAL13_1    253
#define   IADC_CAL13_2    254
#define   IADC_CAL13_3    255
#define   IADC_CAL14_0    256
#define   IADC_CAL14_1    257
#define   IADC_CAL14_2    258
#define   IADC_CAL14_3    259

// DAC noise 
#define   DACOFF      280
#define   DACNUM       15

#define   IDAC0       280        // DAC on first ADC (#0)
#define   IDAC1       281
#define   IDAC2       282
#define   IDAC3       283
#define   IDAC4       284
#define   IDAC5       285
#define   IDAC6       286
#define   IDAC7       287
#define   IDAC8       288
#define   IDAC9       289
#define   IDAC10      290
#define   IDAC11      291
#define   IDAC12      292
#define   IDAC13      293
#define   IDAC14      294

// CSR of ADCs
#define   CSROFF      300
#define   CSRNUM       15

#define   ICSR0       300        // CSR on first ADC (#0)
#define   ICSR1       301
#define   ICSR2       302
#define   ICSR3       303
#define   ICSR4       304
#define   ICSR5       305
#define   ICSR6       306
#define   ICSR7       307
#define   ICSR8       308
#define   ICSR9       309
#define   ICSR10      310
#define   ICSR11      311
#define   ICSR12      312
#define   ICSR13      313
#define   ICSR14      314

// Raw Scalers 
#define   SCAOFF      340
#define   SCANUM        4       // 4 units of 32 channels

#define   ISCALER0_0   340      // first scaler
#define   ISCALER0_1   341
#define   ISCALER0_2   342
#define   ISCALER0_3   343
#define   ISCALER0_4   344
#define   ISCALER0_5   345
#define   ISCALER0_6   346
#define   ISCALER0_7   347
#define   ISCALER0_8   348
#define   ISCALER0_9   349
#define   ISCALER0_10  350
#define   ISCALER0_11  351
#define   ISCALER0_12  352
#define   ISCALER0_13  353
#define   ISCALER0_14  354
#define   ISCALER0_15  355
#define   ISCALER0_16  356
#define   ISCALER0_17  357
#define   ISCALER0_18  358
#define   ISCALER0_19  359
#define   ISCALER0_20  360
#define   ISCALER0_21  361
#define   ISCALER0_22  362
#define   ISCALER0_23  363
#define   ISCALER0_24  364
#define   ISCALER0_25  365
#define   ISCALER0_26  366
#define   ISCALER0_27  367
#define   ISCALER0_28  368
#define   ISCALER0_29  369
#define   ISCALER0_30  370
#define   ISCALER0_31  371

#define   ISCALER1_0   372     // second scaler
#define   ISCALER1_1   373
#define   ISCALER1_2   374
#define   ISCALER1_3   375
#define   ISCALER1_4   376
#define   ISCALER1_5   377
#define   ISCALER1_6   378
#define   ISCALER1_7   379
#define   ISCALER1_8   380
#define   ISCALER1_9   381
#define   ISCALER1_10  382
#define   ISCALER1_11  383
#define   ISCALER1_12  384
#define   ISCALER1_13  385
#define   ISCALER1_14  386
#define   ISCALER1_15  387
#define   ISCALER1_16  388
#define   ISCALER1_17  389
#define   ISCALER1_18  390
#define   ISCALER1_19  391
#define   ISCALER1_20  392
#define   ISCALER1_21  393
#define   ISCALER1_22  394
#define   ISCALER1_23  395
#define   ISCALER1_24  396
#define   ISCALER1_25  397
#define   ISCALER1_26  398
#define   ISCALER1_27  399
#define   ISCALER1_28  400
#define   ISCALER1_29  401
#define   ISCALER1_30  402
#define   ISCALER1_31  403

#define   ISCALER2_0   404     // third scaler
#define   ISCALER2_1   405
#define   ISCALER2_2   406
#define   ISCALER2_3   407
#define   ISCALER2_4   408
#define   ISCALER2_5   409
#define   ISCALER2_6   410
#define   ISCALER2_7   411
#define   ISCALER2_8   412
#define   ISCALER2_9   413
#define   ISCALER2_10  414
#define   ISCALER2_11  415
#define   ISCALER2_12  416
#define   ISCALER2_13  417
#define   ISCALER2_14  418
#define   ISCALER2_15  419
#define   ISCALER2_16  420
#define   ISCALER2_17  421
#define   ISCALER2_18  422
#define   ISCALER2_19  423
#define   ISCALER2_20  424
#define   ISCALER2_21  425
#define   ISCALER2_22  426
#define   ISCALER2_23  427
#define   ISCALER2_24  428
#define   ISCALER2_25  429
#define   ISCALER2_26  430
#define   ISCALER2_27  431
#define   ISCALER2_28  432
#define   ISCALER2_29  433
#define   ISCALER2_30  434
#define   ISCALER2_31  435

#define   ISCALER3_0   436      // fourth scaler
#define   ISCALER3_1   437
#define   ISCALER3_2   438
#define   ISCALER3_3   439
#define   ISCALER3_4   440
#define   ISCALER3_5   441
#define   ISCALER3_6   442
#define   ISCALER3_7   443
#define   ISCALER3_8   444
#define   ISCALER3_9   445
#define   ISCALER3_10  446
#define   ISCALER3_11  447
#define   ISCALER3_12  448
#define   ISCALER3_13  449
#define   ISCALER3_14  450
#define   ISCALER3_15  451
#define   ISCALER3_16  452
#define   ISCALER3_17  453
#define   ISCALER3_18  454
#define   ISCALER3_19  455
#define   ISCALER3_20  456
#define   ISCALER3_21  457
#define   ISCALER3_22  458
#define   ISCALER3_23  459
#define   ISCALER3_24  460
#define   ISCALER3_25  461
#define   ISCALER3_26  462
#define   ISCALER3_27  463
#define   ISCALER3_28  464
#define   ISCALER3_29  465
#define   ISCALER3_30  466
#define   ISCALER3_31  467

// Calibrated Scalers 
#define   SCCOFF       480

#define   ICALSCA0_0   480      // first scaler
#define   ICALSCA0_1   481
#define   ICALSCA0_2   482
#define   ICALSCA0_3   483
#define   ICALSCA0_4   484
#define   ICALSCA0_5   485
#define   ICALSCA0_6   486
#define   ICALSCA0_7   487
#define   ICALSCA0_8   488
#define   ICALSCA0_9   489
#define   ICALSCA0_10  490
#define   ICALSCA0_11  491
#define   ICALSCA0_12  492
#define   ICALSCA0_13  493
#define   ICALSCA0_14  494
#define   ICALSCA0_15  495
#define   ICALSCA0_16  496
#define   ICALSCA0_17  497
#define   ICALSCA0_18  498
#define   ICALSCA0_19  499
#define   ICALSCA0_20  500
#define   ICALSCA0_21  501
#define   ICALSCA0_22  502
#define   ICALSCA0_23  503
#define   ICALSCA0_24  504
#define   ICALSCA0_25  505
#define   ICALSCA0_26  506
#define   ICALSCA0_27  507
#define   ICALSCA0_28  508
#define   ICALSCA0_29  509
#define   ICALSCA0_30  510
#define   ICALSCA0_31  511

#define   ICALSCA1_0   512     // second scaler
#define   ICALSCA1_1   513
#define   ICALSCA1_2   514
#define   ICALSCA1_3   515
#define   ICALSCA1_4   516
#define   ICALSCA1_5   517
#define   ICALSCA1_6   518
#define   ICALSCA1_7   519
#define   ICALSCA1_8   520
#define   ICALSCA1_9   521
#define   ICALSCA1_10  522
#define   ICALSCA1_11  523
#define   ICALSCA1_12  524
#define   ICALSCA1_13  525
#define   ICALSCA1_14  526
#define   ICALSCA1_15  527
#define   ICALSCA1_16  528
#define   ICALSCA1_17  529
#define   ICALSCA1_18  530
#define   ICALSCA1_19  531
#define   ICALSCA1_20  532
#define   ICALSCA1_21  533
#define   ICALSCA1_22  534
#define   ICALSCA1_23  535
#define   ICALSCA1_24  536
#define   ICALSCA1_25  537
#define   ICALSCA1_26  538
#define   ICALSCA1_27  539
#define   ICALSCA1_28  540
#define   ICALSCA1_29  541
#define   ICALSCA1_30  542
#define   ICALSCA1_31  543

#define   ICALSCA2_0   544     // third scaler
#define   ICALSCA2_1   545
#define   ICALSCA2_2   546
#define   ICALSCA2_3   547
#define   ICALSCA2_4   548
#define   ICALSCA2_5   549
#define   ICALSCA2_6   550
#define   ICALSCA2_7   551
#define   ICALSCA2_8   552
#define   ICALSCA2_9   553
#define   ICALSCA2_10  554
#define   ICALSCA2_11  555
#define   ICALSCA2_12  556
#define   ICALSCA2_13  557
#define   ICALSCA2_14  558
#define   ICALSCA2_15  559
#define   ICALSCA2_16  560
#define   ICALSCA2_17  561
#define   ICALSCA2_18  562
#define   ICALSCA2_19  563
#define   ICALSCA2_20  564
#define   ICALSCA2_21  565
#define   ICALSCA2_22  566
#define   ICALSCA2_23  567
#define   ICALSCA2_24  568
#define   ICALSCA2_25  569
#define   ICALSCA2_26  570
#define   ICALSCA2_27  571
#define   ICALSCA2_28  572
#define   ICALSCA2_29  573
#define   ICALSCA2_30  574
#define   ICALSCA2_31  575

#define   ICALSCA3_0   576      // fourth scaler
#define   ICALSCA3_1   577
#define   ICALSCA3_2   578
#define   ICALSCA3_3   579
#define   ICALSCA3_4   580
#define   ICALSCA3_5   581
#define   ICALSCA3_6   582
#define   ICALSCA3_7   583
#define   ICALSCA3_8   584
#define   ICALSCA3_9   585
#define   ICALSCA3_10  586
#define   ICALSCA3_11  587
#define   ICALSCA3_12  588
#define   ICALSCA3_13  589
#define   ICALSCA3_14  590
#define   ICALSCA3_15  591
#define   ICALSCA3_16  592
#define   ICALSCA3_17  593
#define   ICALSCA3_18  594
#define   ICALSCA3_19  595
#define   ICALSCA3_20  596
#define   ICALSCA3_21  597
#define   ICALSCA3_22  598
#define   ICALSCA3_23  599
#define   ICALSCA3_24  600
#define   ICALSCA3_25  601
#define   ICALSCA3_26  602
#define   ICALSCA3_27  603
#define   ICALSCA3_28  604
#define   ICALSCA3_29  605
#define   ICALSCA3_30  606
#define   ICALSCA3_31  607


// TIR data from various crates
#define   TIROFF      610
#define   TIRNUM        4
#define   ITIRDATA    610        // TIRDATA
#define   ITIRDATA1   611        // TIRDATA  - 2nd crate
#define   ITIRDATA2   612        // TIRDATA  - 3rd crate
#define   ITIRDATA3   613        // TIRDATA  - 4th crate

// Helicity info from various crates
#define   HELOFF      620
#define   HELNUM        4
#define   IHELICITY   620        // Helicity
#define   IHELICITY1  621        // helicity from 2nd crate
#define   IHELICITY2  622
#define   IHELICITY3  623

// Timeslot info from various crates
#define   TIMOFF      630
#define   TIMNUM        4
#define   ITIMESLOT   630        // timeslot
#define   ITIMESLOT1  631        // timeslot from 2nd crate
#define   ITIMESLOT2  632
#define   ITIMESLOT3  633

// Pairsynch info from various crates
#define   PAROFF       640
#define   PARNUM         4
#define   IPAIRSYNCH   640        // pairsync
#define   IPAIRSYNCH1  641        // pairsync from 2nd crate
#define   IPAIRSYNCH2  642
#define   IPAIRSYNCH3  643

// Timeboard data
#define   TBDOFF       650
#define   TBDNUM         4

#define   ITIMEBOARD   660
#define   ITIMEBOARD1  661
#define   ITIMEBOARD2  662
#define   ITIMEBOARD3  663

#define   IRAMPDELAY   674
#define   IRAMPDELAY1  675
#define   IRAMPDELAY2  676
#define   IRAMPDELAY3  677

#define   IINTEGTIME   680
#define   IINTEGTIME1  681
#define   IINTEGTIME2  682
#define   IINTETTIME3  683

#define   IOVERSAMPLE  690
#define   IOVERSAMPLE1 691
#define   IOVERSAMPLE2 692
#define   IOVERSAMPLE3 693

#define   IPRECDAC     694
#define   IPRECDAC1    695
#define   IPRECDAC2    696
#define   IPRECDAC3    697

#define   IPITADAC     700

// LUMI data
#define   LMIOFF       705
#define   LMINUM         4

#define   ILUMI1R      705
#define   ILUMI1       706
#define   ILUMI2R      707
#define   ILUMI2       708
#define   ILUMI3R      709
#define   ILUMI3       710
#define   ILUMI4R      711
#define   ILUMI4       712












