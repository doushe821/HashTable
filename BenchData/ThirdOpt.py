import numpy as np
import matplotlib.pyplot as mp

data = np.loadtxt('ThirdOptTicks.txt', delimiter=',', skiprows=0)

average = sum(data[0:len(data)]) / len(data)
print(average)
dispersion = np.sum((data[0:len(data)] - average)**2)
error = np.sqrt(dispersion / (len(data) * (len(data) - 1)))
print(error)
relativeError = error / average
print(relativeError)

with open("ThirdOptResult.txt", 'w') as file:
    file.write("Average = ")
    file.write(f"{average}")
    file.write("\nAbsolute error = ")
    file.write(f"{error}")
    file.write("\nRelative error = ")
    file.write(f"{relativeError}")
