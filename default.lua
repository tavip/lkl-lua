pwd = "/"
print ("~~~~~~~~~~~~~~ begin lua code. Respect my authoritah!")
filename = "default.lua"
a = lkl.stat(filename)
print (a["size"])
for i,v in pairs(a) do
    print (i,v)
end
print ("after printing the table")

ret = lkl.file_rename(pwd .. "test1", pwd .. "test2")
print("file_rename returned", ret)

ret = lkl.file_rename(pwd .. "test2", pwd .. "test1");
print("file_rename returned", ret)

ret = lkl.file_copy("test1", "copydest");
print("file_copy returned", ret)

ret = lkl.dir_make("subdir");
print("dir_make returned", ret)

ret = lkl.dir_make_rec("subdir2/subdir3");
print("dir_make_rec returned", ret)


a = lkl.dir(".")
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
ret = lkl.chdir("/")
print ("chdir returned ", ret)
ret, reason = lkl.getcwd()
print ("getcwd returned ", ret, reason)
ret = lkl.mkdir("lklmkdir")
print ("lkl.mkdir returned ", ret)
print ("~~~ end of the world")
