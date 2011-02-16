function FindProxyForURL(url, host)
{
  // This pac file is for Corus
   if (shExpMatch(host, "*.bnl.gov") ||
       shExpMatch(host, "130.199.*"))
     return "PROXY localhost:3128";
   else if (shExpMatch(host, "localhost*") ||
             shExpMatch(host, "127.0.0.1"))
    return "DIRECT";
   else
     return "PROXY 192.168.1.140:3128";
}

