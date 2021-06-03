% Description: Program gets Training Data

% Before running, upload the 'mpu6050_output' Arduino code to the Uno Board
% Change the COM of this program to your own COM (line 31)
% The sampling rate for training data can be changed on line 39
%   -> setting to a small sampling rates may cause errors
% You may experience errors running the program; rerun it until it works.
%--------------------------------------------------------------------------

% delete any existing timers
listOfTimers = timerfindall;
if ~isempty(listOfTimers)
    delete(listOfTimers(:));
end

% cleaning up MATLAB environment
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

clear all;
counter=1; % continue to get sensor data
rawData = []; % mpu6050 raw data 
save('rawData.mat','rawData');
trainingData = []; 
save('trainingData.mat','trainingData');
temp=[];    % temporarily stores extracted serial port output 
count=1; % visual number to see number of samples extracted

sensor = serial('COM3', 'BaudRate',115200);
fclose(sensor);
fopen(sensor);


% Initalizing and starting Timer
t = timer;
set(t,'executionMode','fixedRate');
set(t,'Period',0.5);    % training data for sampling rate: 500ms
set(t,'StartDelay',2);
t.TimerFcn=@getFeatures; % get sampling feature every 500ms
start(t);
on=1;


% get training samples until reached value indicated in 'getFeatures' 
while true
data=fscanf(sensor);
temp = [str2double(split(data,','))]; % extract and store raw data from serial output 
if counter == 0 % number of training data has been reached
    stop(t);
    fclose(sensor);
    break;
else   % continue to collect mpu6050 sensor data
    rawData = [rawData;temp(1),temp(2),temp(3),temp(4),temp(5),temp(6)];   
end
save('rawData.mat','rawData'); 
end
delete(t)