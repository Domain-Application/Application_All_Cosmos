param([int]$ProcId)
Add-Type @'
using System;using System.Text;using System.Collections.Generic;using System.Runtime.InteropServices;
public class W2{public delegate bool P(IntPtr h,IntPtr l);[DllImport("user32")]public static extern bool EnumWindows(P p,IntPtr l);[DllImport("user32")]public static extern bool EnumChildWindows(IntPtr w,P p,IntPtr l);[DllImport("user32")]public static extern int GetWindowThreadProcessId(IntPtr h,out int pid);[DllImport("user32",CharSet=CharSet.Auto)]public static extern IntPtr SendMessage(IntPtr h,int m,int w,StringBuilder s);[DllImport("user32")]public static extern bool IsWindowVisible(IntPtr h);
public static List<string> Texts(int pid){var r=new List<string>();EnumWindows((h,l)=>{int id;GetWindowThreadProcessId(h,out id);if(id==pid&&IsWindowVisible(h)){var sb=new StringBuilder(4096);SendMessage(h,0x0D,4096,sb);r.Add("[TOP] "+sb);EnumChildWindows(h,(c,x)=>{var s=new StringBuilder(4096);SendMessage(c,0x0D,4096,s);if(s.Length>0)r.Add("   "+s);return true;},IntPtr.Zero);}return true;},IntPtr.Zero);return r;}}
'@
$p = Get-Process -Id $ProcId -ErrorAction SilentlyContinue
if (-not $p) { "PID $ProcId not running"; return }
[W2]::Texts($ProcId)
