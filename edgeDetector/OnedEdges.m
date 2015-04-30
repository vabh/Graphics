clear;

%range of values
finish = 50;
begin = -50;
range = finish - begin + 1;

values = (begin : finish);

%input signals
x = exp(-abs(values));
x = zeros(1, range);
x(70) = 1;
x(30) = 1;

%gloabal values
%alpha
a = 0.6;

ea = exp(-a);
ea2 = ea ^ 2;

%choose the filter to use 
%[1 - smoothing]
%[2 - edge as max]
%[3 - edge as zero crossing]
FILTER = 3;

if FILTER == 1
    S = (1 - ea) ^ 2 / (1 + ea * (a - 1));
elseif FILTER == 2  
    S = (1 - ea)^2 / ea;
else
    S = (1 - ea) / a;
end

%positive
yp = zeros(1, range);
for n = 3 : range
    
    if FILTER == 1
        yp(n) = S * x(n) + S * ea * (a - 1) * x(n - 1) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);
    elseif FILTER == 2
        yp(n) = S * ea * x(n - 1) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);
    else
        yp(n) = x(n) - (1 + S*a) * ea * x(n - 1) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);
    end
end

%negative
yn = zeros(1, range);
for n = (range - 2) : -1 : 1
    
    if FILTER == 1
        yn(n) = S * x(n) + S * ea * (a - 1) * x(n + 1) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);
    elseif FILTER == 2
        yn(n) = S * ea * x(n + 1) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);
    else
        yn(n) = x(n) - (1 + S*a) * ea * x(n + 1) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);
    end
end

%result
y = zeros(1, range);
if FILTER == 1
    S = S;
elseif FILTER == 2
    S = 0;
else
    S = 1;
end

for n = 1 : range
    y(n) = yp(n) + yn(n) - S * x(n);
end

plot(values, x);
hold on;
plot(values, y, 'r');
hold off;