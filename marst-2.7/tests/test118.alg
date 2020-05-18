begin
    integer
        i;
    real
        x;
    array
        a[0, 3];
    i := 1;
    for x := 1.0,
        2.0,
        3.0 do
        begin
        a[i] := x;
        i := i + 1
        end;
end
