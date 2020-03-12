% run it as:
% x= 1:10; 
% aggr_result= pararrayfun(nproc, @monpi, x);

function result = monpi();

result=0;

for i= 1:1e5
	x = rand; y=rand;
	if (x^2 + y^2)<1
		result++;
	endif
endfor

result*=4/1e5; % closer to pi

return;

