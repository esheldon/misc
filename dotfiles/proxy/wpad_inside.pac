function FindProxyForURL(url, host)
{
    return "PROXY localhost:3128";

    // this part is for looking at hadoop 
    if (shExpMatch(host, "astro*.rcf.bnl.gov") 
            || shExpMatch(host,"130.199.184.*"))
        return "PROXY localhost:3128";


    // This pac file is for inside the Lab
    if (shExpMatch(host, "*.bnl.gov") ||
            shExpMatch(host, "130.199.*") ||
            shExpMatch(host, "localhost*") ||
            shExpMatch(host, "127.0.0.1"))
        return "DIRECT";
    else
        return "PROXY 192.168.1.130:3128";
        //return "PROXY 130.199.80.149:3128";
}

