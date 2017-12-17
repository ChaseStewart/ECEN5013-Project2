# ECEN5013-Project2: The Plant Protector
This repository holds the BBG, TIVA , and backend code for the Advanced Practical Embedded Software final project from CU Boulder. We have opted to create the Plant Protector, an embedded IoT project that uses a soil moisture sensor, a temperature sensor, and a light sensor to measure metrics relevant to the life of a plant. 


## Description
The remote sensor/client, a Texas Instruments TIVA board, holds the sensors and communicates via ethernet to the server. The server, a Beaglebone Black, aggregates data from the remote sensor, writes it to a logfile specified at runtime, and opens a connection to a local MySQL database (schema included) that can then be read by a local webserver. The TIVA runs the FreeRTOS operating system and the BeagleboneBlack runs Debian Linux. 


## Features
Some of the features of this project include:
* Intra-process communication using FreeRTOS queues and POSIX Mqueues
* Inter-process communication via TCP packets
* A custom BeagleboneBlack LED driver that is used by the project
* A MySQL database (schema included) that is opened and written to by the BBB
* A custom local webserver on port 8888 that reads from the DB and writes to a highcharts chart.


## Maintainers
This code is written and maintained by [Chase E Stewart](https://github.com/ChaseStewart)  and [Raj Subramaniam](https://github.com/RajKumarSubramaniam)


## Future Work
Although it is outside of the scope of the class, we plan to create an Ansible deployment script in some time to deploy the webserver, install needed modules, and create the webserver so that a user can be ready to test the project with just one or a few commands.

