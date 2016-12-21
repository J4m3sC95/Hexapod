M = csvread('background_noise.csv', 16, 0, [16 0 4322 1]);

X = M(:,1);
t = M(:,2);

L = 4307;                   % Length of signal


T = (t(L) - t(1))/(L*1000);     % Sampling frequency
Fs = 1/T;                   % Sampling period

Y = fft(X);

P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);

f = Fs*(0:(L/2))/L;
plot(f,P1)
title('Single-Sided Amplitude Spectrum of X(t)')
xlabel('f (Hz)')
ylabel('|P1(f)|')