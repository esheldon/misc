fname = File.expand_path("~/.sup/mycontacts.txt")
f = File.new(fname, "r")

email_list = []

f.each_line do |line|
    email_list << line
end
email_list
