%%
% Recording Angles for Radon Transform
%
%%

% clean up:
clear all;
clc;

%%
% setup
% 
% * N: num of recording angles equally distributed in $[0,\pi]$
% * rSAMP: sample distance of r-axis
% * sSAMP: sample distance of s-axis
%
N=5;
rSAMP=0.5;
sSAMP=0.5;

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
r=-1:rSAMP:1;
s=-1:sSAMP:1;

% standard vectors for $r*sigmaT$
line1=sigmasT(:,2)*r;

% iterate over all recording angles
for j=1:N
    sigma=sigmas(:,j); % set current angle

    % create new figure:
    figure;
    hold on;

    %line=[];
    line=zeros(2,N,length(s)); %preallocate matrix
    for i=1:length(s)
        % $ L(s) = s_i*sigma_j + r*sigma_j^{T} $
        line(:,:,i)=[line1(1,:)+s(i)*sigma(1) ; line1(2,:)+s(i)*sigma(2)];

        %plot line and sample-points:
        scatter(line(1,:,i),line(2,:,i));
        plot(line(1,:,i),line(2,:,i));
    end

    % add title with current recording angle to plot:
    str=sprintf('recording angle phi_%d=%d',j,phis(j));
    title(texlabel(str));

    % add grid & origin to plot:
    hline(0,'--r');
    vline(0,'--r');
    grid on;

    % plot angle phi
    angle=(sigma*s)>0;
    plot(angle(1,:),angle(2,:),'k');
end
