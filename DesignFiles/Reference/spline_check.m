% spline_check.m
% Проверка результатов компонента EcoAdvancedMath в MATLAB (или GNU Octave).
% Выводит те же величины, что тесты 1, 2 и 9.2 юнит-теста, для сравнения
% «до N знаков после запятой» и сравнения производительности.
%
% Copyright (c) 2026 Офицеров Д.С.

format long g

%% Тест 1. Пример из документации MATLAB
x  = 0:10;
y  = sin(x);
xq = 0:0.25:10;
yq = spline(x, y, xq);
fprintf('\nТест 1: x = 0:10, y = sin(x)\n');
fprintf('%6s  %22s  %22s\n', 'xq', 'sin(xq)', 'spline(x, y, xq)');
for k = 1:2:numel(xq)
    fprintf('%6.2f  %22.17f  %22.17f\n', xq(k), sin(xq(k)), yq(k));
end
fprintf('max |spline - sin| = %.3e\n', max(abs(yq - sin(xq))));

% Объект pp-формы (аналог IEcoSpline): узлы и коэффициенты
pp = spline(x, y);
[breaks, coefs] = unmkpp(pp);
fprintf('Коэффициенты первых двух кусков [a b c d]:\n');
fprintf('%24.17g%24.17g%24.17g%24.17g\n', coefs(1:2, :).');
% Точный интеграл: сумма первообразных кубических кусков (как метод Integrate компонента)
h = diff(breaks).';
fprintf('Интеграл S от 0 до 10 = %.17g\n', sum(sum(coefs .* [h.^4, h.^3, h.^2, h] ./ repmat([4 3 2 1], numel(h), 1))));

%% Тест 2. Граничные условия «S' задана» (MATLAB: y дополняется наклонами)
xb = [0 0.3 1.1 1.5 2.6 3.0 4.2 5.0];
yb = exp(-xb/2) .* cos(2*xb);
xqb = [-0.5 0 0.15 0.3 0.7 1.1 1.3 2.0 2.6 2.8 3.6 4.2 4.6 5.0 5.5];
yq_nak = spline(xb, yb, xqb);                 % not-a-knot (по умолчанию)
yq_cla = spline(xb, [1.0 yb -0.5], xqb);      % S'(0) = 1, S'(5) = -0.5
fprintf('\nТест 2 (набор B):\n%6s  %22s  %22s\n', 'xq', 'not-a-knot', 'clamped');
for k = 1:numel(xqb)
    fprintf('%6.2f  %22.17f  %22.17f\n', xqb(k), yq_nak(k), yq_cla(k));
end

%% Тест 9.2. Производительность (те же данные, что в юнит-тесте).
% Окно замера 1 с: в Windows cputime меняется шагами 15,6 мс.
fprintf('\n%8s  %16s\n', 'n', 'spline, ms');
for n = [1e3 1e4 1e5 1e6]
    i  = 0:n-1;
    xs = 10 * (i + 0.3 * sin(0.7 * i)) / n;
    ys = sin(xs) .* exp(-xs / 10);
    xqs = 10 * (0:n-1) / n;
    reps = 0; t0 = cputime;
    while cputime - t0 < 1.0
        v = spline(xs, ys, xqs); %#ok<NASGU>
        reps = reps + 1;
    end
    fprintf('%8d  %16.3f\n', n, (cputime - t0) / reps * 1e3);
end
