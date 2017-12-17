# Database Priority-Based IO Using NVMe Devices

This project consists of simulating how the priority-based support provided by NVMe devices
can be exploited to enhance the Quality-of-Service in Database Management Systems.

Our proposal simulate the NVMe Weighted-Round-Robin queues of an NVMe device, as well as
we simulate traces from the different database threads.

## Authors

- Cassiano Campes 
- Dmitry Lachinov

## Simulator CMD args
-i \<input path\> -o \<output path\> -w \<weights path\> -rt \<read time\> - wt \<write time\> -qs \<dev queue size\>

-i path to trace csv

-o path to output file. File either shouldn't exist or be modifiable

-w path to weights csv

-rt page read time (in units)

-wt page write time (in units)

-qs size of the device queue

### Example
-i C:\Users\Dmitry\Documents\GitHub\DBProject\trace.csv -o C:\Users\Dmitry\Documents\GitHub\DBProject\output.csv -w C:\Users\Dmitry\Documents\GitHub\DBProject\weights.csv -rt 10 -wt 100 -qs 10

## Tracer CMD args

-tx \<transaction size\> -cp \<checkpoint size\> -cmp \<compaction size\> -file \<output file\> - req \<number of requests\> -trigger \<compact trigger\>

-tx transaction buffer size (in number of pages)

-cp maximum checkpint size per operation

-cmp maximum compaction size per operation

-file filename to store the simulated traces

-req number of requisitions (traces) to be generated

-trigger trigger value (in # of pages) that starts a compaction service

### Example

~/Develop/DBProject$ ./tracing -tx 100 -cp 5 -cmp 10 -file "parse2.csv" -req 1000000 -trigger 1000
