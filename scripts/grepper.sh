#!/bin/bash

# pipe input into stdin to use
#cat 100.txt | 

# take the formatted sampler data and turn it into a csv
# format is as follows:
# cpu_energy, hdd_energy, bridge_energy, ram_energy, cpu_power, hdd_power, bridge_power, ram_power

sed -r "s/USB.*//g" | 
sed -r "s/POWERSECD.*//" | 

sed -r "s/CPU_Energy \(J\) = (.*)/\1,/g" | 
sed -r "s/HDD_Energy \(J\) = (.*)/\1,/g" |
sed -r "s/Bridge_Energy\(J\) = (.*)/\1,/g" |
sed -r "s/RAM_Energy \(J\) = (.*)/\1,/g" |

sed -r "s/CPU_Average_Power \(W\) \= (.*)/\1,/g" | 
sed -r "s/HDD_Average_Power \(W\) \= (.*)/\1,/g" |
sed -r "s/Bridge_Average_Power\(W\) \= (.*)/\1,/g" |
sed -r "s/RAM_Average_Power \(W\) \= (.*)/\1;/g"  | 

tr '\n' ' ' | 
tr ';' '\n'


#POWERSECD test (1a)
#
#CPU_Energy (J) = 0.006055
#HDD_Energy (J) = 0.001613
#Bridge_Energy(J) = 0.004014
#RAM_Energy (J) = 0.011535
#USB_Energy (J) = 0.000298
#CPU_Average_Power (W) = 2.422040
#HDD_Average_Power (W) = 0.645400
#Bridge_Average_Power(W) = 1.605640
#RAM_Average_Power (W) = 4.613800
#USB_Average_Power (W) = 0.119160
#




