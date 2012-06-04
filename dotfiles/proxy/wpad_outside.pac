function FindProxyForURL(url, host)
{
    // This pac file is for outside of the Lab

    // this part is for looking at hadoop 
    if (shExpMatch(host, "astro*.rcf.bnl.gov") 
            || shExpMatch(host,"130.199.184.*"))
        return "PROXY localhost:3128";

    if (shExpMatch(host, "*.bnl.gov") ||
            shExpMatch(host, "130.199.*"))
        return "PROXY localhost:3128";
    else
        return "DIRECT";
}

