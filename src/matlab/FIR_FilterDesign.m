clc
close all

N=30; %Number of FilterTaps
Fs=17800; %Sampling Frequency
d1 = fdesign.lowpass('N,Fc',N-1,300,Fs); %Fc=cutoff frquency
d2 = fdesign.bandpass('N,Fc1,Fc2',N-1,2500,4500,Fs);
d3 = fdesign.highpass('N,Fc',N-1,4000,Fs);

Hd =window(d1,'window',@kaiser); %In-built Kaiser Window Function to design FIR filters
    s=coeffs(Hd); %In-built function to find filter coefficients
        fprintf(num2str(s.Numerator));
    sum(abs(Hd.Numerator)) %To calculate gain
        fvtool(Hd); %To plot the amg response
     
Hd =window(d2,'window',@kaiser);
    s=coeffs(Hd);            
        fprintf(num2str(s.Numerator));
    sum(abs(Hd.Numerator))
        fvtool(Hd);
    
Hd =window(d3,'window',@kaiser);
    s=coeffs(Hd);
        fprintf(num2str(s.Numerator));
    sum(abs(Hd.Numerator))
        fvtool(Hd);
   
   
   
  
    
   