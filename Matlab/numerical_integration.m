%% Numerische Integration
% Diskretisierung des Integrations-Operators:
% 
% $$ \left(Ax\right)(s) = \int_a^s x(t) dt, \; \forall s \in [a,b] $$
%%

%%
% erstmal aufräumen...
clear all;
clc;

%%
% Nun kommt unsere Funktion $x(t)=(t-1)^2$ als function-handle
f=@(x)(x-1).^2;

%% Einteilung des Intervalls & Stützstellen Berechnung
n=20;
a=-1; % Intervall in den Grenzen [a,b]
b=1;
dist=abs(a)+abs(b);

    %%
    % Einteilung des Intervalls $[a, b]$ in n+1 äquidistante Teile
    % $t_d = \left(t_0, ..., t_n\right)$
t_d=a:dist/n:b;

    %%
    % Bestimmung der Mittelwerte für spätere Funktionsauswertung
    % $$t_n = \left( \frac{t_0+t_1}{2}, ..., \frac{t_{n-1}+t_n}{2} \right)$
t_n=(t_d(1:n) + t_d(2:n+1))/2;

%% Diskretisierung
x_n=feval(f,t_n); % Auswertung von x(t) an Stuetzstellen t_n
A=tril(dist/n*ones(n,n)); % Diskretisierter Operator
y_n=A*x_n'; % Berechnung vom diskreten y
ys_n=y_n.^2; % ... und quadriert

%% Kontinuierliche Funktionen:
% Zum plotten der stetigen Funktion nehmen wir ein feine Auteilung der
% Achse mit 0.001 und berechnen die Daten für die plot-Funktion.
t=a:0.001:b; % wähle möglichst feine Auflösung

    %%
    % $x(t)=(t-1)^2$
x=feval(f,t);

    %% 
    % Stammfunktion $y(t)=\frac{(t-1)^3 + 1}{3}$
    % (Leider funktioniert die direkte Auswertung der Funktion nicht.
    % Daher behelfen wir uns mit der cumtrapz Funktion und einer sehr
    % feinen Auflösung.)
%y=((t-1).^3 + 1)/3;
y=cumtrapz(t,x);
ys=y.^2; %quadriertes Integral

%% Plot
% Als Vergleich plotten wir immer die approximierte zusammen mit der
% kontinuierlichen Funktion.

    %%
    % _x_ und _x_n_
figure
subplot(3,1,1);
hold on;
stairs(t_d,[x_n x_n(n)],'Marker','d','MarkerFaceColor','c');
plot(t,x); % continues x(t)
grid on;

    %%
    % _y_ und _y_n_
subplot(3,1,2);
hold on;
stairs(t_d,[y_n' y_n(n)],'r','Marker','d','MarkerFaceColor','m');
plot(t,y,'r');
grid on;

subplot(3,1,3);
hold on;
stairs(t_d,[ys_n' ys_n(end)],'g','Marker','d','MarkerFaceColor','b');
plot(t,ys,'g');
grid on;