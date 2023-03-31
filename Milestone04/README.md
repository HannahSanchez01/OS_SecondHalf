# OS_SecondHalf
Ryan Morgan
rmorgan7@nd.edu

Noah Corcoran
ncorcora@nd.edu

Hannah Sanchez
hanche5@nd.edu

Kylee Kazenski
kkazensk@nd.edu


Code Testing Results:
Aggressive Locking (10 producers, 3000 iterations)
1 consumer: 76205940, 73392790, 74018624
2: 74747341, 69169806, 76636162
3: 78530072, 76818128, 73549112
4: 80636395, 78871443, 78835973
5: 80630545, 79806685, 77156090
6: 70305949, 73696881, 80237126
7: 78665595, 79571847, 76543757
8: 81295967, 82882521, 83717626
9: 79719044, 80842589, 82138258
10: 81271925, 83442744, 82808806

Condition Variables:
1 consumer: 77266681, 74257758, 73618918
2: 75174122, 72711753, 75679695
3: 80441454, 73882587, 76514862
4: 76653116, 76749425, 75129140
5: 77299985, 75016856, 71098167
6: 77208192, 76725213, 77063871
7: 77571241, 76759145, 69563115
8: 75034224, 79771950, 75015996
9: 74118563, 76268270, 75866771
10: 77691951, 72834552, 75318926

Aggressive locking actually got slower with more consumers, while condition variables stayed roughly the same speed.
This shows that condition variables are better than aggressive locking, although we aren't sure if it's actually supposed to slow down or if that's a quirk of the randomness.