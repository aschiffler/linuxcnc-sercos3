Sercos III Softmaster for Machinekit / LinuxCNC
================================================

## Disclaimer ##
Only for demo purposes! Not to be used in machines, only in controlled safe environments!

## Introduction ##
This implementation is based on the work find here:
[Sercos SoftMaster](https://sourceforge.net/projects/sercos-softmaster-core/files/Sercos%20SoftMaster%20Package/)
and the professionell support of [Dr. May, Mr. Meusert and Mr. Scheurer from Bosch Rexroth AG, Lohr a. Main](http://www.computer-automation.de/steuerungsebene/steuern-regeln/artikel/107001/)
This Code implements two levels additional levels to the stack from the link above (SIII, SICE, CMSD)
1. the RTLX which is the lowest level and makes the connection to the ethernet hardware
2. the S3SM which is the highest and makes the connection to the application in this case to machinekit/Linuxcnc

## Prerequisites ##
- Machinekit runtime and development packages see [here](http://www.machinekit.io/docs/getting-started/install-runtime-packages/)
- ethernet interface (every interface will work since the Softmaster/RTLX level) uses the raw socket access sendto/recvfrom which is provided of the Linux/OS -- see implementation in RTLX_SOCK.c)
- some Sercos III slaves (eg. IndraDrive)

## Install ##
make configure.mk

make

sudo make install

## Usage ## 
(assumes at least oneslave drive)
- halcmd: loadrt s3sm 
//will create thread called sercos-cycle with 2 ms
- halcmd: addf sercos-worker sercos-cycle
- halcmd: start
- halcmd: call sercos-conf f
// Auto detect slaves and configure the Telegrams (with parameter 'f', see the other parameters in the code s3sm_lc_main.c)
-halcmd: setp s3sm.0.op_mode_vel 1
// switch to velocity control
- halcmd: setp s3sm.0.commanded_vel 10
// Value depends on the setting in the drive use the Software IndraWorks to configure the drive
// !Now Enable Drive!
- halcmd: setp s3sm.power_on 1


## features ##
- auto find slaves on bus
- auto config of drives during start up
- Switch between Position Control and Velocitiy Control via HAL Pin
- Demo files in directory config
- Read and Write Parameters via Sercos Service Channel (SVC) during run
- conceptionel test on Raspberry Pi 3 succesful (but not recommended due to the missing eth/phy)

## Drawbacks / Issues ##
- draft implementation
- hardcoded: cycle rate 2 ms, number of slaves (3), configuration of cyclic interchanged values (position, velocity, torque), Ethernet DEV 'eth0' (see RTLX_SOCK.c)
- timing issues: Not yet tested under real conditions
- don't use in real production
- actually min. cycle time of 2 ms seems to be possible
- in detail unclear architecure of threads/semaphores in the SVC when used with the rtapi of machinekit/linuxcnc --> see usage of functions in RTLX_sema.c by SIII_SVCRead 
- Sourcecode from Sourceforge (see link above) "copied" and hand-written makefiles

## Demo / Video ##
[Sercos III Softmaster for Machinekit / Linuxcnc](https://www.youtube.com/watch?v=Sw9DAKn6hoY)
