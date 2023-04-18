# OS_SecondHalf
Ryan Morgan
rmorgan7@nd.edu

Noah Corcoran
ncorcora@nd.edu

Hannah Sanchez
hsanche5@nd.edu

Kylee Kazenski
kkazensk@nd.edu

Performance Tests: OPTIMAL THREADS IS 5 (1 Producer, 4 Consumer)

ncorcora@student10:~/OS/Project04$ time ./redextract TestSmall.pcap -threads 2
Summarizing the processed entries
Parsing of file TestSmall.pcap complete
  Total Packets Parsed:    228
  Total Bytes   Parsed:    131520
  Total Packets Duplicate: 57
  Total Bytes   Duplicate: 64417
  Total Duplicate Percent:  48.98%

real    0m0.012s
user    0m0.006s
sys     0m0.003s

ncorcora@student10:~/OS/Project04$ time ./redextract TestSmall.pcap
Summarizing the processed entries
Parsing of file TestSmall.pcap complete
  Total Packets Parsed:    228
  Total Bytes   Parsed:    131520
  Total Packets Duplicate: 57
  Total Bytes   Duplicate: 64417
  Total Duplicate Percent:  48.98%

real    0m0.008s
user    0m0.007s
sys     0m0.003s

ncorcora@student10:~/OS/Project04$ time ./redextract TestSmall.pcap -threads 8
Summarizing the processed entries
Parsing of file TestSmall.pcap complete
  Total Packets Parsed:    228
  Total Bytes   Parsed:    131520
  Total Packets Duplicate: 57
  Total Bytes   Duplicate: 64417
  Total Duplicate Percent:  48.98%

real    0m0.011s
user    0m0.005s
sys     0m0.007s


ncorcora@student10:~/OS/Project04$ time ./redextract Test.pcap -threads 2
Summarizing the processed entries
Parsing of file Test.pcap complete
  Total Packets Parsed:    79936
  Total Bytes   Parsed:    60198052
  Total Packets Duplicate: 537
  Total Bytes   Duplicate: 377746
  Total Duplicate Percent:   0.63%

real    0m2.440s
user    0m2.264s
sys     0m0.672s

ncorcora@student10:~/OS/Project04$ time ./redextract Test.pcap
Summarizing the processed entries
Parsing of file Test.pcap complete
  Total Packets Parsed:    79936
  Total Bytes   Parsed:    60198052
  Total Packets Duplicate: 537
  Total Bytes   Duplicate: 376768
  Total Duplicate Percent:   0.63%

real    0m2.249s
user    0m2.176s
sys     0m0.478s

ncorcora@student10:~/OS/Project04$ time ./redextract Test.pcap -threads 8
Summarizing the processed entries
Parsing of file Test.pcap complete
  Total Packets Parsed:    79936
  Total Bytes   Parsed:    60198052
  Total Packets Duplicate: 537
  Total Bytes   Duplicate: 377727
  Total Duplicate Percent:   0.63%

real    0m2.430s
user    0m2.311s
sys     0m0.672s





