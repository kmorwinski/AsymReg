%%
% Recording Angles for Radon Transform
%
%%

% clean up:
clear all;
close all; % close all open figures
clc;

%%
% setup
% 
% * N: num of recording angles equally distributed in $[0,\pi]$
% * SAMP: sample time of r-,s-axis
%
N=9;
SAMP=0.25;

%%
% Recording angles
% Following code constructs the rec. angles
% $$ \phis=\left(0°, ..., 180°\right) $$
% $$ \sigmas = \left( ... \right) $$
%
phis=linspace(0,180,N);
phis_=phis*pi/180; % tmp var
sigmas=[cos(phis_);sin(phis_)];
phis_=phis_+pi/2; % +90°
sigmasT=[cos(phis_);sin(phis_)];

% discrete intervalls of $r=[-1,1]$ and $s=[-1,1]$
r=-1:SAMP:1;
s=-1:SAMP:1;

% iterate over all recording angles
for j=1:N
    sigma=sigmas(:,j); % set current angle
    line1=sigmasT(:,j)*r; % standard vectors for $r*sigmaT$

    % create new figure:
    figure;
    hold on;

    %line=[];
    line=zeros(2,length(s),N); %preallocate matrix
    for i=1:length(s)
        % $ L(s) = s_i*sigma_j + r*sigma_j^{T} $
        line(:,:,i)=[line1(1,:)+s(i)*sigma(1) ; line1(2,:)+s(i)*sigma(2)];

        %plot line and sample-points:
        scatter(line(1,:,i),line(2,:,i));
        plot(line(1,:,i),line(2,:,i),'LineWidth',2);
    end

    % add title with current recording angle to plot:
    str=sprintf('recording angle \\phi_%d = %.2f^\\circ',j,phis(j));
    title(str);

    % add grid & origin to plot:
    hline(0,'--r');
    vline(0,'--r');
    grid on;

    % plot angle phi
    angle=(sigma*s);
    angleX=angle(1,:);
    angleY=angle(2,:);
    if (phis(j) <= 90) % 0° <= phi <= 90°
        angleX=angleX(angleX>=0); % only positiv X entries
        angleY=angleY(length(angleY)-length(angleX)+1:end); % choose proper Y
    else
        angleY=angleY(angleY>=0); % only positiv Y entries
        angleX=angleX(length(angleX)-length(angleY)+1:end); % choose proper X
    end
    plot(angleX,angleY,'k','LineWidth',2);
end
