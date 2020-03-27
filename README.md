make with `make.sh`

run with ./main <whatever>

data file format:
1 line (canvas size): `1280 400`
2 line (label): `whatever`
3 line (pixels per second on timeline): `4`
4 line (every n seconds display timestamp): `14`
5 and next lines (block details): `<line number> <start second> <how long> <text>`
where <line number> - number of block line to be drawn on or -1 to place in order
where <text> - length of full line (with 3 numbers in front) is 512 characters. Until newline.

keep any questions about purpose and why this code is not user friendly with yourself.
