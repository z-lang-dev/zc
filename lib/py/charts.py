############################################
# module: charts
############################################

# 饼状图
import matplotlib.pyplot as plt
import numpy as np

def pie(arr):
    y = np.array(arr)
    plt.pie(y)
    plt.show() 

def bar(items, nums, labels, colors):
    fig, ax = plt.subplots()
    ax.bar(items, nums, label=labels, color=colors)
    ax.set_title('Bar Chart')
    ax.legend(title='Color')
    plt.show()