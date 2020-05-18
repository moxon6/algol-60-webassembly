begin
    real
        x integer i;
    array
        a[0 : 3];
    i := 0;
    for x := 1.23,
        2.34,
        3.45 do
        begin
        a[i] := x;
        i := i + 1
        end;
end
