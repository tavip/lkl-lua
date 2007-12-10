
print ("~~~~~~~~~~~~~~ begin lua code. Respect my authoritah!")
filename = "default.lua"
a,v = stat(filename, "type", "size")
print (a,v)
a = lkl.dir(1)
if a == nil then
    print "suck to be a nilval"
else
    t = {}
    print("before insert");
    for i in a do
        table.insert(t,i)
    end
    print("before sort");
    table.sort(t)
    
    for i,v in ipairs(t) do
        print (i,v)
    end
end
a,b = lkl.get_pid()
print ("a=", a, "b=", b)
print ("~~~ end of the world")
