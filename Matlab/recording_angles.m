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
% * rSAMP: sample time of r-axis
% * sSAMP: sample time of s-axis
% * PLOT: show a plot for every every lines-set
% * PRINT: print every lines-set to console
%
N=9;
rSAMP=0.5;
sSAMP=1;
PLOT=false;
PRINT=true;

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

% iterate over all recording angles
for j=1:N
    sigma=sigmas(:,j); % set current angle
    lineOrigin=sigmasT(:,j)*r; % standard vectors for $r*sigmaT$

    if (PLOT)
        % create new figure:
        figure;
        hold on;
    end

    if (PRINT)
        display('********');
        display(sprintf('** recording angle phi_%d = %.2f',j,phis(j)));
    end

    %line=[]; %init matrix w/o preallocation
    lines=zeros(2,length(r),length(s)); %preallocate matrix
    for i=1:length(s)
        % $ L(s) = s_i*sigma_j + r*sigma_j^{T} $
        lines(:,:,i)=[lineOrigin(1,:)+s(i)*sigma(1) ; lineOrigin(2,:)+s(i)*sigma(2)];

        if (PLOT)
            %plot line and sample-points:
            scatter(lines(1,:,i),lines(2,:,i));
            plot(lines(1,:,i),lines(2,:,i),'LineWidth',2);
        end 

        if (PRINT)
            % print line for sample point s:
            %str=;
            display(sprintf('** sample point s_%d = %.2f:',i,s(i)));
            display(lines(:,:,i));
        end
    end
    
    if (PLOT)
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
end
