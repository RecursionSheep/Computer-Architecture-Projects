load('astar.dist');
astar = astar(1 : 80, :);
figure(1);
bar(astar(:, 1), astar(:, 2));
xlabel('dist (/ 5)');
ylabel('number');
load('bzip2.dist');
bzip2 = bzip2(1 : 30, :);
figure(2);
bar(bzip2(:, 1), bzip2(:, 2));
xlabel('dist (/ 5)');
ylabel('number');
load('mcf.dist');
mcf = mcf(1 : 105, :);
figure(3);
bar(mcf(:, 1), mcf(:, 2));
xlabel('dist (/ 5)');
ylabel('number');
load('perlbench.dist');
perlbench = perlbench(1 : 90, :);
figure(4);
bar(perlbench(:, 1), perlbench(:, 2));
xlabel('dist (/ 5)');
ylabel('number');