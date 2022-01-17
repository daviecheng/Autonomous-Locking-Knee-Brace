% Getting training data features

% Change the number of training data to be collected on line 81

function [rawData] = getFeatures(~,~)
load rawData.mat

% count the training data samples
c=evalin('base','count');
c=c+1;
assignin('base','count',c);

% get size of rawData collected during sampling time
dimension = size(rawData);
row= dimension(1);   % get rawData row dimension
column=dimension(2); % get rawData column dimension

% Reset values
accelX_sum=0;
accelY_sum=0;
accelZ_sum=0;
gyroX_sum=0;
gyroY_sum=0;
gyroZ_sum=0;
accelMag_sum=0;
gyroMag_sum=0;

% Sum of accel and gyro individual axis
for sampleLine=1:row
    temp=[rawData(sampleLine,1:column)];
    accelX_sum = accelX_sum + temp(1,1);
    accelY_sum = accelY_sum + temp(1,2);
    accelZ_sum = accelZ_sum + temp(1,3);
    gyroX_sum = gyroX_sum + temp(1,4);
    gyroY_sum = gyroY_sum + temp(1,5);
    gyroZ_sum = gyroZ_sum + temp(1,6);
    accelMag_sum = accelMag_sum + sqrt((temp(1,1)^2) + (temp(1,2)^2) + ...
        (temp(1,3)^2));
    gyroMag_sum = gyroMag_sum + sqrt((temp(1,4)^2) + (temp(1,5)^2) + ...
        (temp(1,6)^2));
end

% Get Mean Accel and Gyro in x,y,z directions
accelX_mean = accelX_sum/row;
accelY_mean = accelY_sum/row;
accelZ_mean = accelZ_sum/row;
gyroX_mean = gyroX_sum/row;
gyroY_mean = gyroY_sum/row;
gyroZ_mean = gyroZ_sum/row;

% Get Mean Accel and Gyro Magnitude
accelMag_mean = accelMag_sum/row;
gyroMag_mean = gyroMag_sum/row;

% Get RMS Accel and Gyro in x,y,z directions
accelX_rms = rms(rawData(1:row,1));
accelY_rms = rms(rawData(1:row,2));
accelZ_rms = rms(rawData(1:row,3));
gyroX_rms = rms(rawData(1:row,4));
gyroY_rms = rms(rawData(1:row,5));
gyroZ_rms = rms(rawData(1:row,6));


% save trainingData in workspace
td=[accelX_mean, accelY_mean, accelZ_mean, gyroX_mean, gyroY_mean, ...
    gyroZ_mean, accelMag_mean, gyroMag_mean, accelX_rms, accelY_rms, ...
    accelZ_rms, gyroX_rms, gyroY_rms, gyroZ_rms];

filename = 'trainingData.mat';
m = matfile(filename,'Writable', true);
trainingData = m.trainingData;
trainingData = [trainingData;td];
save('trainingData.mat','trainingData');
assignin('base','trainingData',trainingData);
clear m;

% Check if the number of training data samples have been reached
td_size=size(trainingData);
td_row=td_size(1);
disp(td_row);
if td_row ~= 20 % gather 20 training samples
    assignin('base','rawData',[]);
    assignin('base','counter',1);
else 
    assignin('base','counter',0);
end
end

