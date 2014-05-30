
clear all;
%close all; % close all open figures
clc;

x0 = zeros(11,11);
z0=[0 0 0 0 0 0 0 0 0 0 0 ;
    0 0 0 0 0 0 0 0 0 0 0 ;
    0 0 0 1 1 1 1 1 0 0 0 ;
    0 0 0 2 2 2 2 2 0 0 0 ;
    0 0 0 1 1 1 1 1 0 0 0 ;
    0 0 0 2 2 2 2 2 0 0 0 ;
    0 0 0 1 1 1 1 1 0 0 0 ;
    0 0 0 2 2 2 2 2 0 0 0 ;
    0 0 0 1 1 1 1 1 0 0 0 ;
    0 0 0 0 0 0 0 0 0 0 0 ;
    0 0 0 0 0 0 0 0 0 0 0 ;
    ];
x0=z0;
x0(3:9,4:8)=x0(3:9,4:8)+.5;
y0=zeros(11,1);

len=11;
trapez=zeros(len,1);
trapez(1) = 1/len * .5;
trapez(2:len-1) = 1/len;
trapez(len) = trapez(1);

for j=1:11
    y0(j)=z0(j,:)*trapez;
end

x=x0;
h=.25;
delta=.3;

for n=1:10
    
    y=zeros(11,1); % init empty radon var.
    % Radon Transform of x
    for j=1:11
        y(j)=x(j,:)*trapez;
    end
    
    diff=y0 - y.^2;
    err=norm(diff)
    if err <= delta
        break;
    end

    part = h*2*y.*diff;
    
    % backproject
    reg = zeros(11,11);
    sigma = [0 1];
    for k=1:11
        for l=1:11
            index=sigma*[k,l]';
            reg(k,l)=part(index);
        end
    end
    
    x = x + reg
end