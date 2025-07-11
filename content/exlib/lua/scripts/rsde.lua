rsde = {
	isempty = function (s)
		return s == nil or s == ''
	end
	
	, s2v = function (s)
		local t = {}
		--s:gsub('%-?%d+', function(n) t[#t+1] = tonumber(n) end)
		for match in s:gmatch("([%d%.%+%-]+),?") do
		  t[#t + 1] = tonumber(match)
		end
		return t
	end
	
	, vector3 = function () 
		local r={0,0,0};
		
		r.inc = function(self,a)
			self[1] = self[1] + a[1]
			self[2] = self[2] + a[2]
			self[3] = self[3] + a[3]
		end
		r.dec = function(self,a)
			self[1] = self[1] - a[1]
			self[2] = self[2] - a[2]
			self[3] = self[3] - a[3]
		end
		r.cpy = function(self,a)
			self[1] = a[1]
			self[2] = a[2]
			self[3] = a[3]
		end
		r.scale = function(self,k)
			self[1] = self[1] * k
			self[2] = self[2] * k
			self[3] = self[3] * k
		end
		r.coord = function(self,k)
			return { self[1] , self[2], self[3]}
		end
		r.sum = function(self,a)
			local s = vector3();
			s:cpy(self);
			s:inc(a);
			
			return s;
		end
		r.dif = function(self,a)
			local s = rsde.vector3();
			s:cpy(self);
			s:dec(a);
			return s;
		end
		r.mul = function(self,k)
			local s = vector3();
			s:cpy(self);
			s:scale(k);
			return s;
		end
		r.mag = function(self)
			return 
				math.sqrt(self[1] * self[1] 
				+ self[2] * self[2] 
				+ self[3] * self[3]) 
		end
		r.cross = function(self,a)
			local ax=self[1]
			local ay=self[2]
			local az=self[3]
			local bx=a[1]
			local by=a[2]
			local bz=a[3]
			local s = rsde.vector3();
			s[1]  = ay*bz-az*by;
			s[2]  = az*bx -ax*bz;
			s[3]  = ax*by-ay*bx;
			return s;
		end	
		r.dot = function(self,a)		
			return self[1]*a[1] +
				self[2]*a[2] +
				self[3]*a[3];
		end	
		r.norma = function(self,a)
			local s = rsde.vector3();
			local mag = self:mag();
			if mag>0.0000001 then
				s[1]  = self[1]/mag;
				s[2]  = self[2]/mag;
				s[3]  = self[3]/mag;
			else
				s[1]  = 1;
				s[2]  = 0;
				s[3]  = 0;
			end
			return s;
		end	
		return r
	end 
	
	, quat_axis = function () 
		local a={}
		a.L = { 1,0,0,0 };
		a.r =rsde.vector3()
		a.do_load = function(self,ini,sect)
			s = ini[sect]; 
			local A = s.A
			
			if( rsde.isempty(A)) then
				self.L = {1,0,0,0}
			else
				a=rsde.s2v(A)
				local f = a(4)*math.pi/180
				local sn = sin (f)
				self.L =  {a(1)*sn, a(2)*sn,a(3)*sn,cos(f)};
			end
			local r = s.r
			if( rsde.isempty(r)) then
				self.r =  rsde.vector3() ;
			else
				local nr=rsde.s2v(r)
				self.r:cpy(  {nr[1]/1000,nr[2]/1000,nr[3]/1000});
			end	
			print(sect .. ": ", self.r)
		end
		return a;
	end 
	
	,node = function ( name,branch )
		local i={
			do_load = function(self,ini) 
				print("dummy load: " .. self.path)
			end
			, do_create = function(self) 
				print("dummy create: " )
			end
			, do_arrange = function(self) 
				print("dummy arrange: ")
			end
			
			, load = function(self,ini)
				if(  rsde.isempty(self.branch) == false )	then
				self.path = self.branch.path .. "." .. self.name
				else
				self.path = self.name
				end
				self:do_load(ini)
				for  k = 1,#self.childs do
					self.childs[k]:load(ini)			
				end
				self.path = ""
			end
			
			, create = function(self)
				self:do_create()
				for  k = 1,#self.childs do
					self.childs[k]:create()			
				end
			end
			
			, arrange = function(self)
				self:do_arrange()
				for  k = 1,#self.childs do
					self.childs[k]:arrange()			
				end
			end
		}
		i.name = name;
		i.branch=branch;
		i.childs={}
		if(  rsde.isempty(branch) == false )	then		
			branch.childs[#branch.childs+1] = i;		
		end
		i.path = {}
		i.shape = 0

		return i;
	end
	
	, a2q = function (n,a)
		local  q={}
		q.x=n.x*sin(a*math.pi/180/2)
		q.y=n.y*sin(a*math.pi/180/2)
		q.z=n.z*sin(a*math.pi/180/2)
		q.w=cos(a*math.pi/180/2)
		return q;
	end

}

return rsde

