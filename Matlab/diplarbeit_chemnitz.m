%% Numerische Integration
% Diskretisierung des Integrations-Operators:
% 
% $$ \left(Ax\right)(s) = \int_0^s x(t) dt $$
%%

%%
% erstmal aufräumen...
clear all;
clc;

%% Einteilung des Intervalls & Stützstellen Berechnung
n=6;
    %%
    % Einteilung des Intervalls $[0, 1]$ in n+1 äquidistante Teile
    % $t_d = \left(t_0, ..., t_n\right)$
t_d=0:1/n:1;
    %%
    % Bestimmung der Mittelwerte für spätere Funktionsauswertung
    % $$t_n = \left( \frac{t_0+t_1}{2}, ..., \frac{t_{n-1}+t_n}{2} \right)$$
t_n=(t_d(1:n) + t_d(2:n+1))/2;

%% Diskretisierung
x_n=(t_n-1).^2; % Auswertung von x(t) an Stuetzstellen t_n
A=tril(1/n*ones(n,n)); % Diskretisierter Operator
y_n=A*x_n'; % Berechnung vom diskreten y

%% Kontinuierliche Funktionen:
% Zum plotten der stetigen Funktion nehmen wir ein feine Auteilung der
% Achse mit 0.001 und berechnen die Daten für die plot-Funktion.
t=0:0.001:1; % wähle möglichst feine Auflösung

    %%
    % $x(t)=(t-1)^2$
x=(t-1).^2;

    %% 
    % Stammfunktion $y(t)=\frac{(t-1)^3 + 1}{3}$
y=((t-1).^3 + 1)/3;

%% Plot
% Als Vergleich plotten wir immer die approximierte zusammen mit der
% kontinuierlichen Funktion.

    %%
    % _x_ und _x_n_
figure
subplot(2,1,1);
hold on;
stairs(t_d,[x_n x_n(n)],'Marker','d','MarkerFaceColor','c');
plot(t,x); % continues x(t)
grid on;

    %%
    % _y_ und _y_n_
subplot(2,1,2);
hold on;
stairs(t_d,[y_n' y_n(n)],'r','Marker','d','MarkerFaceColor','m');
plot(t,y,'r');
grid on;
