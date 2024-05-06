import numpy as np
# fp = open("time.txt", 'r')
# data1 = []
# for line in fp.readlines():
#     data1.append(float(line.strip()))
# # print(data1)

# sorted_delays = np.sort(data1)
# yvals = np.arange(len(sorted_delays))/float(len(sorted_delays))

# for i in range(len(sorted_delays)):
#     print(f"{sorted_delays[i] * 1000} {yvals[i]:.4f}")

fp = open("time2.txt", 'r')
data1 = []
for line in fp.readlines():
    data1.append(float(line.strip()))
# print(data1)

sorted_delays = np.sort(data1)
yvals = np.arange(len(sorted_delays))/float(len(sorted_delays))

for i in range(len(sorted_delays)):
    print(f"{sorted_delays[i] * 1000}    {yvals[i]:.4f}")