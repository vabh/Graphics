clear;

%name of input image
I = imread('test.jpg');
I = rgb2gray(I);
figure(5);
imshow(I);
rows = size(I, 1);
cols = size(I, 2);
opnum = 3;

a = 1;
ea = exp(-a);
ea2 = ea ^ 2;

%Ix
Ix = I;
for i = 1 : rows
    
    S = (1 - ea)^2 / ea;   
    %positive
    yp = Ix(i, :);
    for n = 3 : cols
        yp(n) = S * ea * Ix(i, n - 1) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);
    end

    %negative
    yn = Ix(i, :);
    for n = (cols - 2) : -1 : 1
        yn(n) = S * ea * Ix(i, n + 1) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);
    end

    %result
    y = zeros(1, cols);
    for n = 1 : cols
        y(n) = yp(n) + yn(n);
    end   
    Ix(i,:) = y;
   
end

for i = 1 : rows    

    S = (1 - ea) ^ 2 / (1 + ea * (a - 1));    
    %positive
    yp = Ix(i, :);
    for n = 3 : cols
        yp(n) = S * Ix(i, n) + S * ea * (a - 1) * Ix(i, n - 1) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);        
    end

    %negative
    yn = Ix(i, :);
    for n = (cols - 2) : -1 : 1
        yn(n) = S * Ix(i, n) + S * ea * (a - 1) * Ix(i, n + 1) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);        
    end

    %result
    y = zeros(1, cols);
    for n = 1 : cols
        y(n) = yp(n) + yn(n) - S * Ix(i, n);
    end   
    Ix(i,:) = y;
end

figure(1);
imshow(Ix);
clear yp;
clear yn;
clear y;

%Ixx
Ixx = I;
for i = 1 : rows    
    S = (1 - ea) / a;  
    %positive
    yp = Ixx(i, :);
    for n = 3 : cols        
        yp(n) = Ixx(i, n) - (1 + S*a) * ea * Ixx(i, n - 1) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);
    end

    %negative
    yn = Ixx(i, :);
    for n = (cols - 2) : -1 : 1
        yn(n) = Ixx(i, n) - (1 + S*a) * ea * Ixx(i, n + 1) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);
    end

    %result
    y = zeros(1, cols);
    for n = 1 : cols
        y(n) = yp(n) + yn(n) - Ixx(i, n);
    end   
    Ixx(i,:) = y;
   
end

for i = 1 : rows    
    S = (1 - ea) ^ 2 / (1 + ea * (a - 1));    
    %positive
    yp = Ixx(i, :);
    for n = 3 : cols
        yp(n) = S * Ixx(i, n) + S * ea * (a - 1) * Ixx(i, n - 1) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);        
    end

    %negative
    yn = Ixx(i, :);
    for n = (cols - 2) : -1 : 1
        yn(n) = S * Ixx(i, n) + S * ea * (a - 1) * Ixx(i, n + 1) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);        
    end

    %result
    y = zeros(1, cols);
    for n = 1 : cols
        y(n) = yp(n) + yn(n) - S * Ixx(i, n);
    end   
    Ixx(i,:) = y;
end

figure(2);
imshow(Ixx);
clear yp;
clear yn;
clear y;

%Iy
Iy = I;
for i = 1 : cols
    S = (1 - ea) ^ 2 / (1 + ea * (a - 1));    
    %positive
    yp = Iy(:, i);
    for n = 3 : rows
        yp(n) = S * Iy(n, i) + S * ea * (a - 1) * Iy(n - 1, i) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);        
    end

    %negative
    yn = Iy(:, i);
    for n = (rows - 2) : -1 : 1
        yn(n) = S * Iy(n, i) + S * ea * (a - 1) * Iy(n + 1, i) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);        
    end

    %result
    y = zeros(1, rows);
    for n = 1 : rows
        y(n) = yp(n) + yn(n) - S * Iy(n, i);
    end   
    Iy(:, i) = y;
end

for i = 1 : cols
   
    S = (1 - ea)^2 / ea;
    %positive
    yp = Iy(:, i);
    for n = 3 : rows        
        yp(n) = S * ea * Iy(n - 1, i) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);
    end

    %negative
    yn = Iy(:, i);
    for n = (rows - 2) : -1 : 1        
        yn(n) = S * ea * Iy(n + 1, i) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);
    end

    %result
    y = zeros(1, rows);
    for n = 1 : rows
        y(n) = yp(n) + yn(n);
    end   
    Iy(:, i) = y;
end

figure(3);
imshow(Iy);
clear yp;
clear yn;
clear y;

%Iyy
Iyy = I;
for i = 1 : cols
    
    S = (1 - ea) ^ 2 / (1 + ea * (a - 1));    
    %positive
    yp = Iyy(:, i);
    for n = 3 : rows
        yp(n) = S * Iyy(n, i) + S * ea * (a - 1) * Iyy(n - 1, i) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);        
    end

    %negative
    yn = Iyy(:, i);
    for n = (rows - 2) : -1 : 1
        yn(n) = S * Iyy(n, i) + S * ea * (a - 1) * Iyy(n + 1, i) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);        
    end

    %result
    y = zeros(1, rows);
    for n = 1 : rows
        y(n) = yp(n) + yn(n) - S * Iyy(n, i);
    end   
    Iyy(:, i) = y;
end

for i = 1 : cols   
    S = (1 - ea) / a;
    %positive
    yp = Iyy(:, i);
    for n = 3 : rows        
        yp(n) = Iyy(n, i) - (1 + S*a) * ea * Iyy(n - 1, i) + 2 * ea * yp(n - 1) - ea2 * yp(n - 2);
    end

    %negative
    yn = Iyy(:, i);
    for n = (rows - 2) : -1 : 1        
        yn(n) = Iyy(n, i) - (1 + S*a) * ea * Iyy(n + 1, i) + 2 * ea * yn(n + 1) - ea2 * yn(n + 2);
    end

    %result
    y = zeros(1, rows);
    for n = 1 : rows
        y(n) = yp(n) + yn(n) - Iyy(n, i);
    end   
    Iyy(:, i) = y;
end

figure(4);
imshow(Iyy);
clear yp;
clear yn;
clear y;

Ifinal = zeros(rows, cols);

threshold = 50;

for i = 1 : rows
    for j = 1 : cols
        val = (Ix(i,j)^2 + Iy(i,j)^2);
        k = sqrt(double(val));
        if(k > threshold)
            %checking for local maxima along eta direction
            theta = atan(Iy(i,j)/Ix(i,j));
            eta = [cos(theta) sin(theta); -sin(theta) cos(theta)];
            
            pt1 = [0;1];
            pt1 = eta*pt1;
            pt2 = [0;-1];
            pt2 = eta*pt2;
            
            lefti = round(i + pt1(1,1));
            leftj = round(j + pt1(2,1));
            righti = round(i + pt2(1,1));
            rightj = round(j + pt2(2,1));
            
            if(lefti > 0 && lefti <= r && righti > 0 && righti <= r && leftj > 0 && leftj <= c && rightj > 0 && rightj <= c)
                if (sqrt( 1.0 * (Ix(lefti,leftj)^2 + Iy(lefti,leftj)^2) ) < k && sqrt( 1.0 * (Ix(righti,rightj)^2 + Iy(righti,rightj)^2) )<k)
                    Ifinal(i,j) = 255;
                end
            end
        end
    end
end