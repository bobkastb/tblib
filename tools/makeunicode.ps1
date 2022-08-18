
#  Формат файла UnicodeData.txt ftp://ftp.unicode.org/Public/3.2-Update/UnicodeData-3.2.0.html#Case%20Mappings

$basefolder = "D:\work\BOB\baselib\tblib";
$crlf="
";
$config= @{
	downloadfolder = "$basefolder\locale\unicode-data";
	cppdir="$basefolder\locale\unicode.inc"
}
$ctx = @{
	maps=@{ 'C'=@{}; 'T'=@{}; 'F'=@{}; 'S'=@{};};
	mapFull=@{};
	minmaxD_Cmap=@();
	densMap=@{}
	IndexValLimits=@(-(0x7fff), 30000);
	SpecCase_index=@{};
	SpecCase_list= @();
}
$global:ctx = $ctx;

function makezeroarray($l) { 1..$l | %{0} };

function test_minmax( $mapC , $mapFull ) {
	$mmc = $ctx.IndexValLimits;
	$mm=@(0,0,0,0);
	foreach ($k in $mapC.keys) {
		$v = $mapC[$k]; if (!($v -is [int])) { continue; }
		$v = $v - $k;
		if ($v -lt $mm[0]) { $mm[0]=$v
		} elseif ($v -gt $mm[1]) { $mm[1]=$v }
		if ( $v -lt $mmc[0]) {$mm[2]++}
		if ( $v -ge $mmc[1]) {$mm[3]++}
	}
	$ctx.minmaxD_Cmap = $mm;
	write-host 'minmax:',$mm
}
function hcf_ClosureMaps( $ctx ){
	$mf = $ctx.mapFull; $keys = $mf.keys -as [object[]];
	#$ctx.mapFull.values | select -first 3 | %{ write-host $_, $_.gettype() , ($_ -is [int]) }
	while(1){
		$changes=0;
		foreach ($k in $keys){
			$mf[$k]= $mf[$k] | %{ $n=$mf[$_]; if ($n) {$changes++;$n} else {$_}  }
		}
		write-host "changes = $changes"
		if ($changes -eq 0 ) { break; }
	};	
	#$ctx.mapFull.values | select -first 3 | %{ write-host $_, $_.gettype() , ($_ -is [int]) }
}

function hcf_DeCompositeMaps( $ctx ){
	#$ctx.mapFull.values | where {$_ -is [int]} |select -first 3 | %{ write-host $_, $_.gettype() , ($_ -is [int]) }
	$mf = $ctx.mapFull; 
	$m_C=@{}; $m_f=@{};
	foreach ($k in $mf.keys) {
		$v= $mf[$k];
		if ($v -is [int]) {$m_C[$k]=$v} else {$m_F[$k]=$v}
	}
	$ctx.maps['C']=$m_C;
	$ctx.maps['F']=$m_F;
}
function hcf_TestDencity( $ctx ){
	$meas= $ctx.mapFull.keys | measure -Max -Min
	write-host ("hcf_TestDencity() MaxV={0}" -f $meas.Maximum)
	$dmaps=@{}; $minsize=4*$meas.Maximum; $bestdmap=$null;
	function hcf_TestDencityB( $bcnt ){
		$m=@{}; $asize = [int][Math]::Pow(2, $bcnt);
		$dmaps[$bcnt] = $dmap = @{ bits=$bcnt; mapI=$m; asize=$asize; cntindex=1+[int](($meas.Maximum+1) / $asize); index=@(); trm=@() };
		$mf = $ctx.mapFull;
		$mf.keys| %{ $m[ [int]($_ / $dmap.asize) ]++; }
		$dmap.cnt= $m.count;
		$dmap.sizeptr = if ($dmap.cnt -lt 256) {1} else {2};
		$dmap.size = ($dmap.cnt * $dmap.asize)*4 + $dmap.cntindex*$dmap.sizeptr;
		$ctx.densMap = if ($dmap.size -lt $minsize) { $dmap } else {$ctx.densMap};
		
		write-host ("bits {0}: c:{1}({2}) s:{3}" -f $dmap.bits,$dmap.cnt, $dmap.cntindex , $dmap.size)
	}
	#$dia = 4..13;
	$dia = 6; #debug
	foreach ($b in $dia){ 
		hcf_TestDencityB $b
	}	
	#Make arrays
	$dmap = $ctx.densMap;
	$mf = $ctx.mapFull;
	
	$ptr=1; 
	$dmap.index = for ($i=0;$i -lt $dmap.cntindex;$i++ ) {
		if ($dmap.mapI[$i]) { $ptr; $ptr++; } else { 0 }; }

	#write-host "ptr=$ptr","MAPI",$dmap.mapI.keys; 
	
	$dmap.trm = makezeroarray $ptr;	
	$szA = $dmap.asize;
	for ($i=0;$i -lt $ptr;$i++){
		$dmap.trm[$i] = makezeroarray $szA
	}
	$dmap.ExtraTable=@{ cnttab=0; };
	$mf.keys| sort | %{ $key=$_; $i=[int]($_ / $szA); $ofs= $_ % $szA;
		$ia = $dmap.index[$i];
		$v= $val = $mf.$_;
		if (!($val -is [int])) { 
			$v= $dmap.ExtraTable.cnttab; $dmap.ExtraTable.cnttab++;
			$dmap.ExtraTable[$v] = @{ k=$key; v=$val };
			if ($val.count -gt 3) { throw "Invalid case folding len!" }
			$v = $v + ($val.count * [Math]::Pow(2, 30) )
		};
		$dmap.trm[$ia][$ofs]= $v;
	}	
	# $mf.keys | sort 
}

function hcf_MakeCpp( $ctx ){
	$file = $config.cppdir + "\Inc_Unicode_CaseFolding.cpp" ;
	$dmap = $ctx.densMap; $szA = $dmap.asize;
	#for ($i=0;$i -lt )
	$i=1; $buff=@(); 
	$buff += "enum{{ {0} COUNTBIT_cft={1}; {0} }};" -f $crlf,$dmap.bits;
	$buff += "static uint8_t casef_Indexes[]={";
	$buff += ($dmap.index | %{ "0x{0:x}" -f $_; if (!($i % 32)) { $crlf;}; $i++; } ) -join ", ";
	$buff += "};";
	$buff += "static char32_t casefolding[][$szA]={";
	$f = " ";
	$buff += $dmap.trm | %{ $v= $_; $f+"{"+ (($v  | %{"0x{0:x}" -f [int64]$_ })  -join ", ") + "}"; $f = ","; }
	$buff += "};";
	$ext = $dmap.ExtraTable;
	$buff += "static char32_t expand_casefolding[][3]={";
	$f = " ";
	$buff += for ( $i=0;$i -lt $ext.cnttab; $i++ ) {
		$em= $ext[$i]; 
		$a = ($em.v | %{"0x{0:x}" -f [int64]$_ }) -join ", ";
		"{0}{{ {1} }}; //CHAR {2} (0x{3:x})" -f $f,$a, [char]$em.k  , $em.k;
		$f = ",";
	}
	$buff += "};";
	$buff | out-file $file -Encoding 'utf8'
}

function Handle_CaseFolding() { 
	write-host "**** Handle_CaseFolding ****"
	$maps=$ctx.maps;
	(get-content ($config.downloadfolder+"\CaseFolding.txt") ) | where {$_[0] -ne '#'} |  %{ $a=$_ -split '; '; 
		if ($a.length -lt 2) {
		}elseif ($a.length -lt 3) {
			write-host "error parsing:$_",$a.length
		} else {
			$m = $maps[ $a[1] ];
			#$a | out-host
			$m[ [system.Convert]::ToInt32($a[0],16)] = $a[2];#[system.Convert]::ToInt32($a[2],16);
		}	
	}
	$maps.keys | %{  write-host ("$_ : {0}" -f $maps[$_].count ) }
	$mapFull=$ctx.mapFull;
	$mapC=$maps['C'];
	foreach ($k in ($mapC.keys -as [object[]])) {
		$mapFull[$k] = $mapC[$k] = [system.Convert]::ToInt32($mapC[$k],16)
	}
	$mapF=$maps['F'];
	foreach ($k in ($mapF.keys -as [object[]])) {
		$a = $mapF[$k].trim() -split " " | %{ [system.Convert]::ToInt32( $_ ,16) };
		$mapFull[$k] = $mapF[$k] = $a;
	}
	#test_minmax $mapC;
	write-host "hcf_ClosureMaps()"
	hcf_ClosureMaps $ctx 
	write-host "hcf_DeCompositeMaps()"
	hcf_DeCompositeMaps $ctx 
	test_minmax $mapC $ctx.mapFull;
	hcf_TestDencity $ctx
	hcf_MakeCpp $ctx
};

function Handle_SpecialCasing() { 
	$cont = (get-content ($config.downloadfolder+"\SpecialCasing.txt") ) | where {$_[0] -ne '#'} ;
	# [system.Convert]::ToInt32($a[0],16)
	$ctx.SpecCase_index=@{};
	$ctx.SpecCase_list= $list = foreach ($s in $cont) { 
		if (! ($s -match "([^#]*)(#.*)")) { continue; }
		if( !$matches[1].Trim()) {continue}
		$comment= $matches[2];
		$a=$matches[1] -split '; '; 
		0..3 | %{ $i=$_; $a[$i] = $a[$i] -split ' ' | where {$_.Trim()} | %{ [system.Convert]::ToInt32($_,16); }; }
		$r=@{ code= $a[0]; lower= $a[1]; title= $a[2]; upper= $a[3]; condition = $a[4]; comment= $comment; }
		$ctx.SpecCase_index[$r.code] = $r;
		$r;
	}	
}


function do_TestDencity( $ctx , $keyslist , $dia ){
	$meas= $keyslist | measure -Max -Min
	write-host ("TestDencity() MaxV={0}" -f $meas.Maximum)
	$dmaps=@{};  $bestdmap=$null;
	$res = @{ minsize=4*$meas.Maximum }
	function do_TestDencityB( $bcnt ){
		$m=@{}; $asize = [int][Math]::Pow(2, $bcnt);
		$dmaps[$bcnt] = $dmap = @{ bits=$bcnt; mapI=$m; asize=$asize; cntindex=1+[int](($meas.Maximum+1) / $asize); index=@(); trm=@() };
		$keyslist | %{ $m[ [int]($_ / $dmap.asize) ]++; }
		$dmap.cnt= $m.count;
		$dmap.sizeptr = if ($dmap.cnt -lt 256) {1} else {2};
		$dmap.size = ($dmap.cnt * $dmap.asize)*4 + $dmap.cntindex*$dmap.sizeptr;
		$res = if ($dmap.size -lt $res.size) { $dmap } else { $res };
		
		write-host ("bits {0}: c:{1}({2}) s:{3}" -f $dmap.bits,$dmap.cnt, $dmap.cntindex , $dmap.size)
	}

	if (!$dia) $dia = 4..13;
	$ $dia = 6; #debug
	foreach ($b in $dia){ 
		do_TestDencityB $b
	}	
	#Make arrays
	$dmap = $res;
	
	$ptr=1; # init index array for rows 
	$dmap.index = for ($i=0;$i -lt $dmap.cntindex;$i++ ) {
		if ($dmap.mapI[$i]) { $ptr; $ptr++; } else { 0 }; }

	#write-host "ptr=$ptr","MAPI",$dmap.mapI.keys; 
	
	# init trm array
	$dmap.trm = makezeroarray $ptr;	
	$szA = $dmap.asize;
	for ($i=0;$i -lt $ptr;$i++){
		$dmap.trm[$i] = makezeroarray $szA
	}
	
	return $res;
}


function hud_Pack( $ctx ){
	$dia = 4..13;
	$mf = UnicodeData_index;
	
	$dmap.ExtraTable=@{ cnttab=0; };
	$mf.values | %{	$r= $_;
		$r.transfer = if ( $r.special ) { 
			$v= $dmap.ExtraTable.cnttab; $dmap.ExtraTable.cnttab++;
			$dmap.ExtraTable[$v] = $r;
			$v + [Math]::Pow(2, 31) ;
		} else {
			if ( $r.lower ) {  
				$r.lower + [Math]::Pow(2, 30); 
			} else {  
				$r.upper;	}
		}
	}
	$ctx.densMapUD = $dmap = do_TestDencity $ctx  (UnicodeData_index.keys -as object[]) $dia 
	
	$mf.keys| sort | %{ 
		$key=$_; $i=[int]($key / $dmap.asize ); $ofs= $key % $dmap.asize;
		$ia = $dmap.index[$i];
		$v= $mf.$_.transfer;
		$dmap.trm[$ia][$ofs]= $v;
	}	
}

function hud_MakeCpp( $ctx ){
	$file = $config.cppdir + "\Inc_Unicode_CaseData.cpp" ;
	$dmap = $ctx.densMapUD; $szA = $dmap.asize;
	#for ($i=0;$i -lt )
	$i=1; $buff=@(); 
	$buff += "enum{{ {0} COUNTBIT_hud={1}; {0} }};" -f $crlf,$dmap.bits;
	$buff += "static uint8_t hud_Indexes[]={";
	$buff += ($dmap.index | %{ "0x{0:x}" -f $_; if (!($i % 32)) { $crlf;}; $i++; } ) -join ", ";
	$buff += "};";
	$buff += "static char32_t hud_CaseTransfer[][$szA]={";
	$f = " ";
	$buff += $dmap.trm | %{ $v= $_; $f+"{"+ (($v  | %{"0x{0:x}" -f [int64]$_ })  -join ", ") + "}"; $f = ","; }
	$buff += "};";
	$ext = $dmap.ExtraTable;
	$buff += "static char32_t hud_ExpandCaseTransfer[][3]={";
	$f = " ";
	#TODO:
	$buff += "};";
	$buff | out-file $file -Encoding 'utf8'
}

function Handle_UnicodeData() { 
	$iUC=12;$iLC=13;$iTC=14;$iType=2;
	$cont = (get-content ($config.downloadfolder+"\UnicodeData.txt") ) | where {$_[0] -ne '#'} ;
	$ctx.UnicodeData_index=@{};
	$ctx.UnicodeData_list= $list=foreach ($s in $cont){
		$a = $s -split ';'
		if (!$a[$iUC] -and !$a[$iLC] -and !$a[$iTC]) { continue; } #no casemapping
		0,12,13,14 | %{ $i=$_; $a[$i] = if ($a[$i]) { [system.Convert]::ToInt32($a[$i],16); } else { 0 } }
		$r = @{	code=$a[0];	upper=$a[$iUC];	lower=$a[$iLC];	title=$a[$iTC];	}
		$tc = $a[$iTC];
		$r.typec = if ($tc -eq 'Lu') { 'u' 
		}elseif ($tc -eq 'Ll') { 'l' 
		}elseif ($tc -eq 'Lt') { 't' 
		}else {
			write-host "warning unhandled type char $($r.code)"; continue;
		}
		if ($r.title) {
		   $r.special = $r.title -and [bool]( 'upper','lower' | where ($r.$_ -and ($r.$_ -ne $r.title)) );
		}
		$ctx.UnicodeData_index[$r.code] = $r;
		if ($r.special) {
			$ctx.SpecCase_index[$r.code] = $r;
		}	
		$r;
	}
	Handle_SpecialCasing;
	hud_Pack $ctx
	hud_MakeCpp $ctx
}
$config | out-host
Handle_CaseFolding;