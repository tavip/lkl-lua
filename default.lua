
print ("~~~~~~~~~~~~~~ begin lua code. Respect my authoritah!")
filename = "default.lua"

a = apr.dir(".")
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

a,b = lkl.get_pid()
print ("a=", a, "b=", b)
print ("~~~ end of the world")