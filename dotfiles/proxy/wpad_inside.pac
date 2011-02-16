function FindProxyForURL(url, host)
{
  // This pac file is for inside the Lab
   if (shExpMatch(host, "*.bnl.gov") ||
       shExpMatch(host, "130.199.*") ||
       shExpMatch(host, "localhost*") ||
       shExpMatch(host, "127.0.0.1"))
     return "DIRECT";
   else
     return "PROXY 192.168.1.130:3128";
}

