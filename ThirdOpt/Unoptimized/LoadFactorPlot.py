import csv
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# Указываем путь к CSV-файлу
csv_file = 'LoadFactor.csv'  # Замените на ваш файл

data = []

# Чтение CSV-файла
with open(csv_file, 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        data.append(float(row[1]))  

x = list(range(0, len(data)))  # От 0 до 4096 включительно

# Построение гистограммы
plt.figure(figsize=(12, 6))
plt.bar(x, data, width=1.0, align='edge')

# Настройка отображения
plt.xlim(0, len(data))
plt.title("MurMur distribution")
plt.xlabel("Bucket number")
plt.ylabel("Value")

# Оптимизация производительности для большого числа столбцов
plt.tight_layout()

word = 0
quadsum = 0
for i in range(0,len(data)):
    word += data[i]
print(word)
loadFactor = word / len(data)
print("Load factor = {}".format(loadFactor))
for i in range(0, len(data)):
    quadsum += (data[i] - loadFactor)**2
disperion = np.sqrt(quadsum / ((len(data))*(len(data) - 1)))
print("Dispersion = {}".format(loadFactor * disperion))
# Сохранение или отображение
plt.savefig('histogram.png', dpi=150)
plt.show()