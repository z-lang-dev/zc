############################################
# module: charts
############################################

# 饼状图
import matplotlib.pyplot as plt
import numpy as np

def pie(a, b, c, d):
    y = np.array([a, b, c, d])
    plt.pie(y)
    plt.show() 