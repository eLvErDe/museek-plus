def extract(code, data):
	start = data.find("/*")
	end = data.find("*/")
	desc, d_in, d_out = data[start+2:end].split("\n\t\n")
	desc = desc.split(" -- ")
	print desc
	r = """
<a name="%s"/>
<table class='message' cellspacing="0" cellpadding="0">
 <tr>
  <td class="messagecode">%s</td>
  <td class="messageshort">%s</td>
  <td class="messagedesc">%s</td>
 </tr><tr><td class="message" colspan="3">
  <table class="message_in">
""" % (code, code, desc[0][2:], desc[1])
 	
 	s = d_in.split("\n")
 	for i in s:
 		t = i.split(" -- ")
 		if len(t) == 1:
 			t.append("")
 		r += """   <tr><td class="message_in">%s</td><td class="message_in_desc">%s</td></tr>\n""" % (t[0][1:], t[1])

 	r += """  </table><table class="message_out">\n"""

 	s = d_out.split("\n")
 	for i in s:
 		t = i.split(" -- ")
 		if len(t) == 1:
 			t.append("")
 		r += """   <tr><td class="message_out">%s</td><td class="message_out_desc">%s</td></tr>""" % (t[0][1:], t[1])
 	
	r += """
  </table>
 </td></tr>
</table>
"""
	return r
	

def gendoc(header, template):
	import re

	regexp = re.compile("^IFACEMESSAGE\\(.*, (.*)\\)", re.M)
	d = open(header).read()
	t = open(template).read()

	start = 0
	body = ""
	while 1:
		s = regexp.search(d[start:])
		if not s:
			break
		start = start + s.end()
		body += extract(s.groups()[0], d[start:])
	return t.replace("@BODY@", body)
