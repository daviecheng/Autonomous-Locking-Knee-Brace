# Python code to get C Code for Random Forest Algorithm

import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score,confusion_matrix
from sklearn.metrics import plot_confusion_matrix

# Read the csv file
Train = pd.read_csv(r'C:\Users\David\Desktop\chest TD.csv')

# X_data is features, Y_data is label
X_data=Train.iloc[:,:14]
Y_data = Train.iloc[:,14]

# RandomForestClassifier
ML = RandomForestClassifier(30,max_depth=10).fit(X_data,Y_data) # 30 decision trees

# convert Random Forest model to C code
from micromlgen import port
c_code=port(ML, classmap = {0:0,1:1})
print(c_code)