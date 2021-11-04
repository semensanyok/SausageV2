for f in $(ls);do 
is_64=$(objdump -f $f | grep architecture | grep "x86-64")
if [[ $is_64 ]]; then
mv $f x64/
echo $f
fi
done